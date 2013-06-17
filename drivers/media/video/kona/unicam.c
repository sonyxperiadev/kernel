/*****************************************************************************
* Copyright 2006 - 2009 Broadcom Corporation.  All rights reserved.
*
* Unless you and Broadcom execute a separate written software license
* agreement governing use of this software, this software is licensed to you
* under the terms of the GNU General Public License version 2, available at
* http://www.broadcom.com/licenses/GPLv2.php (the "GPL").
*
* Notwithstanding the above, under no circumstances may you combine this
* software in any way with any other Broadcom software provided under a
* license other than the GPL, without Broadcom's express prior written
* consent.
*****************************************************************************/

#include <linux/slab.h>
#include <linux/interrupt.h>
#include <linux/platform_device.h>
#include <linux/dma-mapping.h>

#include "unicam.h"


/*
 * unicam_isr - interrupt service routine for unicam module.
 * @irq: not used
 * @_unicam: pointer to the unicam device
 *
 * Handles the correesponding callback
 *
 * Returns IRQ_HANDLED when IRQ was handled or IRQ_NONE when the
 * IRQ wasn't handled.
 */
static irqreturn_t unicam_isr(int irq, void *_unicam)
{
	struct unicam_device *unicam = _unicam;

	kona_unicam_csi2_isr(&unicam->csi2a);

	return IRQ_HANDLED;
}

/* --------------------------------------------------------------------------
 * Pipeline power management operations
 *
 * Entities must be powered up when part of a pipeline that contains at least
 * one open video device node.
 *
 * To achieve this use the entity use_count field to track the number of users.
 * for entities corresponding to video device nodes the use_count files stores
 * the user count of the node. For entities corresponding to subdevs the
 * use_count files stores the total number of users of all video device nodes
 * in the pipeline.
 *
 * The unicam_pipeline_pm_use() function must be called in the open and close
 * handlers of video device nodes. It increments and decrements the use count
 * of all subdev entities in the pipeline.
 *
 * To react to link management on powered pipelines, the link setup
 * notification callback updates the use count of all enities in the source and
 * sink sides of the link.
 */

/*
 * unicam_pipeline_pm_use_count - count the number of users of a pipeline
 * @entity: the entity
 *
 * return the total numbers of users of all video device nodes in the pipeline.
 */
static int unicam_pipeline_pm_use_count(struct media_entity *entity)
{
	struct media_entity_graph graph;
	int use = 0;

	media_entity_graph_walk_start(&graph, entity);

	while ((entity = media_entity_graph_walk_next(&graph))) {
		if (media_entity_type(entity) == MEDIA_ENT_T_DEVNODE)
			use += entity->use_count;
	}

	return use;
}


/*
 * unicam_pipeline_pm_power_one - apply power change to an entity
 * @entity - the entity
 * @change : use count change
 *
 * Change the entity use count by @change. If the entity is a subdev update its
 * power state by calling the core::s_power operation when the use count goes
 * from 0 to != 0 from !=0 to 0.
 *
 * return 0 on success or a negative error code on failure
 */

static int unicam_pipeline_pm_power_one(struct media_entity *entity, int change)
{
	struct v4l2_subdev *subdev;

	subdev = media_entity_type(entity) == MEDIA_ENT_T_V4L2_SUBDEV
		? media_entity_to_v4l2_subdev(entity) : NULL;

	if (entity->use_count == 0 && change > 0 && subdev != NULL) {
		int ret;

		ret = v4l2_subdev_call(subdev, core, s_power, 1);
		if (ret < 0 && ret != -ENOIOCTLCMD)
			return ret;
	}

	entity->use_count += change;
	WARN_ON(entity->use_count < 0);

	if (entity->use_count == 0 && change < 0 && subdev != NULL)
		v4l2_subdev_call(subdev, core, s_power, 0);

	return 0;
}

/*
 * unicam_pipeline_pm_power - apply power change to all entities in a pipeline
 * @entity: the entity
 * @change: use count change
 *
 * Walk the pipeline to update the use count and power state of all non node
 * entities.
 *
 * return 0 on success or a negative error code on failure.
 */
static int unicam_pipeline_pm_power(struct media_entity *entity, int change)
{
	struct media_entity_graph graph;
	struct media_entity *first = entity;
	int ret = 0;

	if (!change)
		return 0;

	media_entity_graph_walk_start(&graph, entity);

	while (!ret && (entity = media_entity_graph_walk_next(&graph)))
		if (media_entity_type(entity) != MEDIA_ENT_T_DEVNODE)
			ret = unicam_pipeline_pm_power_one(entity, change);

	if (!ret)
		return 0;

	media_entity_graph_walk_start(&graph, first);

	while ((first = media_entity_graph_walk_next(&graph))
			&& first != entity)
		if (media_entity_type(first) != MEDIA_ENT_T_DEVNODE)
			unicam_pipeline_pm_power_one(first, -change);

	return ret;
}

/*
 * kona_unicam_pipeline_pm_use - update the use count of an entity
 * @entity: the entiy
 * @use: use(1) or stop using(1) the entity
 *
 * Update the use count of all entities in the pipeline and power entities on
 * or off accordingly
 *
 * return 0 on success or a negative error code on failure. powering entities
 * off is assumed to never fail. no failure can occur when the use parameter is
 * set to 0.
 */
int kona_unicam_pipeline_pm_use(struct media_entity *entity, int use)
{
	int change = use ? 1 : -1;
	int ret;

	mutex_lock(&entity->parent->graph_mutex);

	/* apply the use count to node */
	entity->use_count += change;
	WARN_ON(entity->use_count < 0);

	/* apply the power change to connected non nodes */
	ret = unicam_pipeline_pm_power(entity, change);
	if (ret < 0)
		entity->use_count -= change;

	mutex_unlock(&entity->parent->graph_mutex);

	return ret;
}


/*
 * unicam_pipeline_link_notify - link management notification callback
 * @source: pad at the start of the link
 * @sink: pad at the end of the link
 * @flags: new link flags that will be applied
 *
 * React to link management on powered pipelines by updating the use count of
 * all entities in the source and sink sides of the link. Entities are powered
 * on and off accordingly.
 *
 * Returns 0 on success or a negative error code on failue. Powering entities
 * off is assumed to never fail. This function will not fail for disconnection
 * events
 */
static int unicam_pipeline_link_notify(struct media_pad *source,
		struct media_pad *sink, u32 flags)
{
	int ret;
	int source_use = unicam_pipeline_pm_use_count(source->entity);
	int sink_use = unicam_pipeline_pm_use_count(sink->entity);

	if (!(flags & MEDIA_LNK_FL_ENABLED)) {
		/* power off entities */
		unicam_pipeline_pm_power(source->entity, -sink_use);
		unicam_pipeline_pm_power(sink->entity, -source_use);
		return 0;
	}

	ret = unicam_pipeline_pm_power(source->entity, sink_use);
	if (ret < 0)
		return ret;

	ret = unicam_pipeline_pm_power(sink->entity, source_use);
	if (ret < 0)
		unicam_pipeline_pm_power(source->entity, -sink_use);

	return ret;
}

/*
 * unicam_pipeline_enable - enable streaming on a pipeline
 * @pipe: unicam pipeline
 * @mode: stream mode (single shot or continuous)
 *
 * Walk the entities chain starting at the pipeline output video node and start
 * all module in the chain in the given node.
 *
 * Return 0 if successful, or the return value of the failed video::s_stream
 * operation otherwise.
 */
static int unicam_pipeline_enable(struct unicam_pipeline *pipe,
		enum unicam_pipeline_stream_state mode)
{
	struct media_entity *entity;
	struct media_pad *pad;
	struct v4l2_subdev *subdev;
	int ret;

	entity = &pipe->output->video.entity;
	while (1) {
		pad = &entity->pads[0];
		if (!(pad->flags & MEDIA_PAD_FL_SINK))
			break;

		pad = media_entity_remote_source(pad);
		if (pad == NULL ||
		media_entity_type(pad->entity) != MEDIA_ENT_T_V4L2_SUBDEV)
			break;

		entity = pad->entity;
		subdev = media_entity_to_v4l2_subdev(entity);

		ret = v4l2_subdev_call(subdev, video, s_stream, mode);
		if (ret < 0 && ret != -ENOIOCTLCMD)
			return ret;

	}
	return 0;
}

/*
 * unicam_pipeline_disable - disable streaming on a pipeline
 * @pipe: unicam pipeline
 *
 * Walk the entities chain starting at the pipeline out video note and stop
 * all the modules in the chain
 */

static int unicam_pipeline_disable(struct unicam_pipeline *pipe)
{
	struct media_entity *entity;
	struct media_pad *pad;
	struct v4l2_subdev *subdev;
	int failure = 0;

	entity = &pipe->output->video.entity;
	while (1) {
		pad = &entity->pads[0];
		if (!(pad->flags & MEDIA_PAD_FL_SINK))
			break;

		pad = media_entity_remote_source(pad);
		if ((pad == NULL) ||
		media_entity_type(pad->entity) != MEDIA_ENT_T_V4L2_SUBDEV)
			break;

		entity = pad->entity;
		subdev = media_entity_to_v4l2_subdev(entity);

		failure = v4l2_subdev_call(subdev, video, s_stream, 0);
	}

	return 0;
}
/*
 * kona_unicam_pipeline_set_stream - enable/disable streaming on a pipeline
 * @pipe: unicam pipeline
 * @state: stream state (stopped, single shot or continuous)
 *
 * set the pipeline to the given stream state. pipelines can be started in
 * single or continous mode
 *
 * return 0 if successful or the return value of the failed video::s_stream
 * operations otherwise. The pipeline state is not updated when the operation
 * fails, except when stopping the pipeline.
 */

int kona_unicam_pipeline_set_stream(struct unicam_pipeline *pipe,
		enum unicam_pipeline_stream_state state)
{
	int ret;
	if (state == UNICAM_PIPELINE_STREAM_STOPPED)
		ret = unicam_pipeline_disable(pipe);
	else
		ret = unicam_pipeline_enable(pipe, state);

	if (ret == 0 || state == UNICAM_PIPELINE_STREAM_STOPPED)
		pipe->stream_state = state;

	return ret;
}

/*-----------------------------------------------------
 * clock management apis
 * ----------------------------------------------------
 */

/*
 * unicam_enable_clocks - Enable unicam clocks
 * @unicam: KONA Unicam device
 * Return 0 if successful, or clk_enable return value
 */
static int unicam_enable_clocks(struct unicam_device *unicam)
{
	int r;

	r = pi_mgr_dfs_request_update(&unicam->dfs_client, PI_OPP_NORMAL);
	if (r) {
		dev_err(unicam->dev, "failed to update dfs request(NORMAL)\n");
		return r;
	}

	r = clk_enable(unicam->csi2_axi_clk);
	if (r) {
		dev_err(unicam->dev, "clk_enable csi2_axi_clk failed\n");
		goto err_clk_enable;
	}

	return 0;

err_clk_enable:
	if (pi_mgr_dfs_request_update(&unicam->dfs_client,
				PI_MGR_DFS_MIN_VALUE))
		dev_err(unicam->dev, "failed to update dfs request(min)\n");

	return r;
}

/*
 * unicam_disable_clocks - Disable unicam clocks
 * @unicam: KONA Unicam device
 */
static void unicam_disable_clocks(struct unicam_device *unicam)
{
	int r;
	clk_disable(unicam->csi2_axi_clk);
	r = pi_mgr_dfs_request_update(&unicam->dfs_client,
			PI_MGR_DFS_MIN_VALUE);
	if (r)
		dev_err(unicam->dev, "failed to update dfs request(min)\n");
}

static void unicam_put_clocks(struct unicam_device *unicam)
{
	int r;

	if (unicam->csi2_axi_clk) {
		clk_put(unicam->csi2_axi_clk);
		unicam->csi2_axi_clk = NULL;
	}

	if (unicam->dfs_client.valid) {
		r = pi_mgr_dfs_request_remove(&unicam->dfs_client);
		if (r)
			dev_err(unicam->dev, "failed to remove dfs request\n");

	}
}

static int unicam_get_clocks(struct unicam_device *unicam)
{
	int ret;

	/* dfs request */
	ret = pi_mgr_dfs_add_request(&unicam->dfs_client, "kona-unicam-mc",
			PI_MGR_PI_ID_MM, PI_MGR_DFS_MIN_VALUE);
	if (ret < 0) {
		dev_err(unicam->dev, "cound not register pi dfs client\n");
		return ret;
	}

	unicam->csi2_axi_clk = clk_get(unicam->dev, "csi0_axi_clk");
	if (IS_ERR_OR_NULL(unicam->csi2_axi_clk)) {
		dev_err(unicam->dev, "unable to get clock csi0_axi_clk\n");
		unicam_put_clocks(unicam);
		return PTR_ERR(unicam->csi2_axi_clk);
	}

	return 0;
}

/*
 * kona_unicam_get - Acquire unicam resource
 *
 * Initializes the clocks for the first acquire
 *
 * Increment the reference count on the unicam. If the first reference is
 * taken, enable clocks and power up
 *
 * Return a pointer to the unicam device structure, or NULL if an error
 * occured.
 */
struct unicam_device *kona_unicam_get(struct unicam_device *unicam)
{
	struct unicam_device *__unicam = unicam;

	if (unicam == NULL)
		return NULL;

	mutex_lock(&unicam->unicam_mutex);
	if (unicam->ref_count > 0)
		goto out;

	if (unicam_enable_clocks(unicam) < 0) {
		__unicam = NULL;
		goto out;
	}

out:
	if (__unicam != NULL)
		unicam->ref_count++;
	mutex_unlock(&unicam->unicam_mutex);

	return __unicam;
}

/*
 * kona_unicam_put - Release the unicam
 *
 * Decrement the reference count on the unicam. if the last reference is
 * released, disbale the clocks
 */

void kona_unicam_put(struct unicam_device *unicam)
{
	if (unicam == NULL)
		return;

	mutex_lock(&unicam->unicam_mutex);
	BUG_ON(unicam->ref_count == 0);
	if (--unicam->ref_count == 0)
		unicam_disable_clocks(unicam);
	mutex_unlock(&unicam->unicam_mutex);
}

/*------------------------------------------------------------
 * Operations related to media controller/v4l2 registrations
 * -----------------------------------------------------------
 */


static void unicam_unregister_entities(struct unicam_device *unicam)
{
	kona_unicam_csi2_unregister_entities(&unicam->csi2a);
	v4l2_device_unregister(&unicam->v4l2_dev);
	media_device_unregister(&unicam->media_dev);
}

/*
 * unicam_register_subdev_group - register a group of subdevices
 * @unicam: KONA unicam device
 * @board_info: i2c subdevs board information array
 *
 * register all i2c subdevices in the board_info array.
 *
 * return a pointer to the sensor media entity if it has been successfully
 * registered, or null otherwise.
 */

static struct v4l2_subdev *
unicam_register_subdev_group(struct unicam_device *unicam,
		struct unicam_subdev_i2c_board_info *i2c_info)
{
	struct v4l2_subdev *sensor = NULL;
	struct i2c_adapter *adapter;

	if (i2c_info == NULL)
		return NULL;

	adapter = i2c_get_adapter(i2c_info->i2c_adapter_id);
	if (adapter == NULL) {
		dev_err(unicam->dev, "unabled to get i2c adapter %d for"
				"device %s\n", i2c_info->i2c_adapter_id,
				i2c_info->board_info.type);
		return NULL;
	}

	sensor = v4l2_i2c_new_subdev_board(&unicam->v4l2_dev, adapter,
			&i2c_info->board_info, NULL);

	return sensor;
}

static int unicam_register_entities(struct unicam_device *unicam)
{
	struct unicam_platform_data *pdata = unicam->pdata;
	struct unicam_v4l2_subdevs_groups *subdevs;
	int ret, i;

	/* media device registration */
	unicam->media_dev.dev = unicam->dev;
	strlcpy(unicam->media_dev.model, "Broadcom Kona Unicam",
			sizeof(unicam->media_dev.model));
	unicam->media_dev.hw_revision = unicam->revision;
	unicam->media_dev.link_notify = unicam_pipeline_link_notify;
	ret = media_device_register(&unicam->media_dev);
	if (ret < 0) {
		dev_err(unicam->dev, "media device registration failed (%d)\n",
				ret);
		return ret;

	}

	/* v4l2 device registration */
	unicam->v4l2_dev.mdev = &unicam->media_dev;
	ret = v4l2_device_register(unicam->dev, &unicam->v4l2_dev);
	if (ret < 0) {
		dev_err(unicam->dev, "V4L2 device registration failed (%d)\n",
				ret);
		goto done;
	}

	/* now register all enitites */
	ret = kona_unicam_csi2_register_entities(&unicam->csi2a,
			&unicam->v4l2_dev);
	if (ret < 0) {
		dev_err(unicam->dev, "failed to register csi2a entities (%d)\n",
				ret);
		goto done;
	}

	/* now register external entities */
	for (i = 0; i < pdata->num_subdevs; i++) {
		struct v4l2_subdev *sensor;
		struct media_entity *input;
		unsigned int pad;

		subdevs = &pdata->subdevs[i];

		sensor = unicam_register_subdev_group(unicam,
				subdevs->i2c_info);
		if (sensor == NULL)
			continue;

		sensor->host_priv = subdevs;

		/*
		 * connect the sensor to the correct interface module.
		 * we only have one receiver here
		 */
		switch (subdevs->interface) {
		case UNICAM_INTERFACE_CSI2_PHY1:
			input = &unicam->csi2a.subdev.entity;
			pad = CSI2_PAD_SINK;
			break;

		default:
			dev_err(unicam->dev, "invalid interface type %u\n",
					subdevs->interface);
			goto done;
		}

		ret = media_entity_create_link(&sensor->entity, 0, input, pad,
				0);

		if (ret < 0)
			goto done;
	};

	ret = v4l2_device_register_subdev_nodes(&unicam->v4l2_dev);

done:
	if (ret < 0)
		unicam_unregister_entities(unicam);

	return ret;
}

static int unicam_initialize_modules(struct unicam_device *unicam)
{
	int ret;
	ret = kona_unicam_csi2_init(unicam);
	if (ret < 0)
		dev_err(unicam->dev, "unicam csi2 initialization failed\n");

	return ret;
}

static void unicam_cleanup_modules(struct unicam_device *unicam)
{
	kona_unicam_csi2_cleanup(unicam);
}

static int unicam_probe(struct platform_device *pdev)
{
	struct unicam_platform_data *pdata = pdev->dev.platform_data;
	struct unicam_device *unicam;
	int ret;

	if (pdata == NULL) {
		dev_err(&pdev->dev, "platform data is null\n");
		return -EINVAL;
	}

	unicam = kzalloc(sizeof(*unicam), GFP_KERNEL);
	if (!unicam) {
		dev_err(&pdev->dev, "could not allocate memory\n");
		return -ENOMEM;
	}

	mutex_init(&unicam->unicam_mutex);

	unicam->dev = &pdev->dev;
	unicam->pdata = pdata;
	unicam->ref_count = 0;

	unicam->raw_dmamask = DMA_BIT_MASK(32);
	unicam->dev->dma_mask = &unicam->raw_dmamask;
	unicam->dev->coherent_dma_mask = DMA_BIT_MASK(32);


	platform_set_drvdata(pdev, unicam);

	ret = unicam_get_clocks(unicam);
	if (ret < 0) {
		dev_err(unicam->dev, "failed to get unicam clocks\n");
		goto err_clks;
	}

	if (kona_unicam_get(unicam) == NULL)
		goto err_clks;

	/* TODO - fix this by reading version from unicam register */
	unicam->revision = 0x1;
	/*
	 * map resources here after removing csl/chal layer
	 */
	/* unicam_map_mem_resource(pdev, unicam); */

	/* interrupt */
	unicam->irq_num = platform_get_irq(pdev, 0);
	if (unicam->irq_num <= 0) {
		dev_err(unicam->dev, "no IRQ resource\n");
		ret = -ENODEV;
		goto err_irq;
	}

	if (request_irq(unicam->irq_num, unicam_isr, IRQF_SHARED,
				"KONA Unicam MC",
				unicam)) {
		dev_err(unicam->dev, "unable to request IRQ\n");
		ret = -EINVAL;
		goto err_irq;
	}

	/* Entities */
	ret = unicam_initialize_modules(unicam);
	if (ret < 0) {
		dev_err(unicam->dev, "unable to initialize modules\n");
		goto err_modules;
	}

	ret = unicam_register_entities(unicam);
	if (ret < 0) {
		dev_err(unicam->dev, "unable to register entities\n");
		goto err_register;
	}

	kona_unicam_put(unicam);

	return 0;

err_register:
	unicam_cleanup_modules(unicam);
err_modules:
	free_irq(unicam->irq_num, unicam);
err_irq:
	kona_unicam_put(unicam);
err_clks:
	unicam_put_clocks(unicam);
	platform_set_drvdata(pdev, NULL);
	mutex_destroy(&unicam->unicam_mutex);
	kfree(unicam);

	return ret;
}

static int unicam_remove(struct platform_device *pdev)
{
	struct unicam_device *unicam = platform_get_drvdata(pdev);

	unicam_unregister_entities(unicam);
	unicam_cleanup_modules(unicam);

	free_irq(unicam->irq_num, unicam);
	unicam_put_clocks(unicam);

	kfree(unicam);
	return 0;
}

static struct platform_driver unicam_driver = {
	.probe		= unicam_probe,
	.remove		= unicam_remove,
	.driver = {
		.owner	= THIS_MODULE,
		.name	= "kona-unicam-mc",
	},
};

static int __init unicam_init(void)
{
	return platform_driver_register(&unicam_driver);
}

static void __exit unicam_exit(void)
{
	platform_driver_unregister(&unicam_driver);
}

late_initcall(unicam_init);
module_exit(unicam_exit);

MODULE_DESCRIPTION("Broadcom Kona UNICAM driver");
MODULE_AUTHOR("Pradeep Sawlani <spradeep@broadcom.com>");
MODULE_LICENSE("GPL");
MODULE_VERSION("0.0.1");

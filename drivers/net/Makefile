#
# Makefile for the Linux network device drivers.
#

#
# Networking Core Drivers
#
obj-$(CONFIG_BONDING) += bonding/
obj-$(CONFIG_DUMMY) += dummy.o
obj-$(CONFIG_EQUALIZER) += eql.o
obj-$(CONFIG_IFB) += ifb.o
obj-$(CONFIG_MACVLAN) += macvlan.o
obj-$(CONFIG_MACVTAP) += macvtap.o
obj-$(CONFIG_DE600) += de600.o
obj-$(CONFIG_DE620) += de620.o
obj-$(CONFIG_LANCE) += lance.o
obj-$(CONFIG_SUN3_82586) += sun3_82586.o
obj-$(CONFIG_SUN3LANCE) += sun3lance.o
obj-$(CONFIG_DEFXX) += defxx.o
obj-$(CONFIG_SGISEEQ) += sgiseeq.o
obj-$(CONFIG_SGI_O2MACE_ETH) += meth.o
obj-$(CONFIG_AT1700) += at1700.o
obj-$(CONFIG_EL1) += 3c501.o
obj-$(CONFIG_EL16) += 3c507.o
obj-$(CONFIG_ELMC) += 3c523.o
obj-$(CONFIG_IBMLANA) += ibmlana.o
obj-$(CONFIG_ELMC_II) += 3c527.o
obj-$(CONFIG_EL3) += 3c509.o
obj-$(CONFIG_3C515) += 3c515.o
obj-$(CONFIG_EEXPRESS) += eexpress.o
obj-$(CONFIG_EEXPRESS_PRO) += eepro.o
obj-$(CONFIG_8139CP) += 8139cp.o
obj-$(CONFIG_8139TOO) += 8139too.o
obj-$(CONFIG_ZNET) += znet.o
obj-$(CONFIG_CPMAC) += cpmac.o
obj-$(CONFIG_DEPCA) += depca.o
obj-$(CONFIG_EWRK3) += ewrk3.o
obj-$(CONFIG_ATP) += atp.o
obj-$(CONFIG_NI5010) += ni5010.o
obj-$(CONFIG_NI52) += ni52.o
obj-$(CONFIG_NI65) += ni65.o
obj-$(CONFIG_ELPLUS) += 3c505.o
obj-$(CONFIG_AC3200) += ac3200.o 8390.o
obj-$(CONFIG_APRICOT) += 82596.o
obj-$(CONFIG_LASI_82596) += lasi_82596.o
obj-$(CONFIG_SNI_82596) += sni_82596.o
obj-$(CONFIG_MVME16x_NET) += 82596.o
obj-$(CONFIG_BVME6000_NET) += 82596.o
obj-$(CONFIG_SC92031) += sc92031.o
obj-$(CONFIG_NET_ISLAND) += island_net.o

# This is also a 82596 and should probably be merged
obj-$(CONFIG_LP486E) += lp486e.o

obj-$(CONFIG_ETH16I) += eth16i.o
obj-$(CONFIG_ZORRO8390) += zorro8390.o
obj-$(CONFIG_HPLANCE) += hplance.o 7990.o
obj-$(CONFIG_MVME147_NET) += mvme147.o 7990.o
obj-$(CONFIG_EQUALIZER) += eql.o
obj-$(CONFIG_KORINA) += korina.o
obj-$(CONFIG_MIPS_JAZZ_SONIC) += jazzsonic.o
obj-$(CONFIG_MIPS_AU1X00_ENET) += au1000_eth.o
obj-$(CONFIG_MIPS_SIM_NET) += mipsnet.o
obj-$(CONFIG_SGI_IOC3_ETH) += ioc3-eth.o
obj-$(CONFIG_DECLANCE) += declance.o
obj-$(CONFIG_ATARILANCE) += atarilance.o
obj-$(CONFIG_A2065) += a2065.o
obj-$(CONFIG_HYDRA) += hydra.o
obj-$(CONFIG_ARIADNE) += ariadne.o
obj-$(CONFIG_CS89x0) += cs89x0.o
obj-$(CONFIG_MACSONIC) += macsonic.o
obj-$(CONFIG_MACMACE) += macmace.o
obj-$(CONFIG_MAC89x0) += mac89x0.o
obj-$(CONFIG_TUN) += tun.o
obj-$(CONFIG_VETH) += veth.o
obj-$(CONFIG_VIRTIO_NET) += virtio_net.o

#
# Networking Drivers
#
obj-$(CONFIG_ARCNET) += arcnet/
obj-$(CONFIG_DEV_APPLETALK) += appletalk/
obj-$(CONFIG_CAIF) += caif/
obj-$(CONFIG_CAN) += can/
obj-$(CONFIG_ETRAX_ETHERNET) += cris/
obj-$(CONFIG_NET_DSA) += dsa/
obj-$(CONFIG_ETHERNET) += ethernet/
obj-$(CONFIG_FDDI) += fddi/
obj-$(CONFIG_HIPPI) += hippi/
obj-$(CONFIG_HAMRADIO) += hamradio/
obj-$(CONFIG_IRDA) += irda/
obj-$(CONFIG_PLIP) += plip/
obj-$(CONFIG_PPP) += ppp/
obj-$(CONFIG_PPP_ASYNC) += ppp/
obj-$(CONFIG_PPP_BSDCOMP) += ppp/
obj-$(CONFIG_PPP_DEFLATE) += ppp/
obj-$(CONFIG_PPP_MPPE) += ppp/
obj-$(CONFIG_PPP_SYNC_TTY) += ppp/
obj-$(CONFIG_PPPOE) += ppp/
obj-$(CONFIG_PPPOL2TP) += ppp/
obj-$(CONFIG_PPTP) += ppp/
obj-$(CONFIG_SLIP) += slip/
obj-$(CONFIG_SLHC) += slip/
obj-$(CONFIG_NET_SB1000) += sb1000.o
obj-$(CONFIG_SUNGEM_PHY) += sungem_phy.o
obj-$(CONFIG_TR) += tokenring/
obj-$(CONFIG_WAN) += wan/
obj-$(CONFIG_WLAN) += wireless/
obj-$(CONFIG_WIMAX) += wimax/

obj-$(CONFIG_VMXNET3) += vmxnet3/
obj-$(CONFIG_XEN_NETDEV_FRONTEND) += xen-netfront.o
obj-$(CONFIG_XEN_NETDEV_BACKEND) += xen-netback/

obj-$(CONFIG_USB_CATC)          += usb/
obj-$(CONFIG_USB_KAWETH)        += usb/
obj-$(CONFIG_USB_PEGASUS)       += usb/
obj-$(CONFIG_USB_RTL8150)       += usb/
obj-$(CONFIG_USB_HSO)		+= usb/
obj-$(CONFIG_USB_USBNET)        += usb/
obj-$(CONFIG_USB_ZD1201)        += usb/
obj-$(CONFIG_USB_IPHETH)        += usb/
obj-$(CONFIG_USB_CDC_PHONET)   += usb/

obj-$(CONFIG_WLAN) += wireless/
#obj-m += wireless/
obj-$(CONFIG_NET_TULIP) += tulip/
obj-$(CONFIG_HAMRADIO) += hamradio/
obj-$(CONFIG_IRDA) += irda/
obj-$(CONFIG_ETRAX_ETHERNET) += cris/
obj-$(CONFIG_ENP2611_MSF_NET) += ixp2000/

obj-$(CONFIG_NETCONSOLE) += netconsole.o
obj-$(CONFIG_BRCM_NETCONSOLE) += brcm_netconsole.o

obj-$(CONFIG_FS_ENET) += fs_enet/

obj-$(CONFIG_NETXEN_NIC) += netxen/
obj-$(CONFIG_NIU) += niu.o
obj-$(CONFIG_VIRTIO_NET) += virtio_net.o
obj-$(CONFIG_SFC) += sfc/

obj-$(CONFIG_WIMAX) += wimax/
obj-$(CONFIG_CAIF) += caif/

obj-$(CONFIG_OCTEON_MGMT_ETHERNET) += octeon/
obj-$(CONFIG_PCH_GBE) += pch_gbe/
obj-$(CONFIG_TILE_NET) += tile/

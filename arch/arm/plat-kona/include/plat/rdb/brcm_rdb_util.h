#ifndef _BRCM_RDB_UTIL_H_
#define _BRCM_RDB_UTIL_H_

/*
** The following two macros create a "combine" routine which can be nested.
** Without the nesting you can't call BRCM_CONCAT(BRCM_CONCAT(x,y),z)
*/

#define BRCM_CONCAT(a,b) BRCM_CONCATX(a,b)
#define BRCM_CONCATX(a,b) a ## b

/*
** HELPER MACROS
** These are used to combine names for the actual read and write macros.
*/

#define BRCM_FIELDNAME(r,f)   BRCM_CONCAT(BRCM_CONCAT(r,_),f)

#define BRCM_REGOFS(r)      BRCM_CONCAT(r,_OFFSET)

#define BRCM_REGTYPE(r)     BRCM_CONCAT(r,_TYPE)

#define BRCM_REGADDR(b,r)   ( (b) + ( BRCM_REGOFS(r) ) )

#define BRCM_FIELDMASK(r,f) (BRCM_CONCAT(BRCM_FIELDNAME(r,f),_MASK))

#define BRCM_FIELDSHIFT(r,f)  (BRCM_CONCAT(BRCM_FIELDNAME(r,f),_SHIFT))

/*
** The following macros read and write registers or bit fields.
**
** b is the base address name (NAME Must be generated from RDB rdb2h.pl)
** r is the register name     (NAME Must be generated from RDB rdb2h.pl)
** f is the field name        (NAME Must be generated from RDB rdb2h.pl)
** d is the data to write.
**
** Reserved bits handling is enforced by the macros:
** 1.  Reserved bit must be written to 0.
** 2.  Reserved bits are undefined when read, so masked off to zero.
**
** Note: Compiler optimizes away AND operation when reserved mask is 0.
*/

#define BRCM_READ_REG(b,r)  ( ( *(volatile BRCM_REGTYPE(r) *) BRCM_REGADDR(b,r) &         \
                                ~( BRCM_CONCAT( r, _RESERVED_MASK) ) ) )

#define BRCM_WRITE_REG(b,r,d) ( ( *(volatile BRCM_REGTYPE(r) *) BRCM_REGADDR(b,r) ) =      \
                                ( (d) & ~( BRCM_CONCAT( r, _RESERVED_MASK)) ) )

#define BRCM_READ_REG_IDX(b,r,i)  ( ( ((volatile BRCM_REGTYPE(r) *) BRCM_REGADDR(b,r))[i] &  \
                                    ~( BRCM_CONCAT( r, _RESERVED_MASK) ) ) )

#define BRCM_WRITE_REG_IDX(b,r,i,d) ( (((volatile BRCM_REGTYPE(r) *) BRCM_REGADDR(b,r))[i] ) \
                                = ( (d) & ~( BRCM_CONCAT( r, _RESERVED_MASK) ) ) )

#define BRCM_READ_REG_FIELD(b,r,f)   ( ( BRCM_READ_REG(b,r) & BRCM_FIELDMASK(r,f) ) >> \
                                       BRCM_FIELDSHIFT(r,f) )

#define BRCM_WRITE_REG_FIELD(b,r,f,d)  (BRCM_WRITE_REG(b,r,                            \
                             (( ((d) << BRCM_FIELDSHIFT(r,f)) & BRCM_FIELDMASK(r,f)) | \
                                (BRCM_READ_REG(b,r) & (~BRCM_FIELDMASK(r,f))  ))))

#endif // _BRCM_RDB_UTIL_H_


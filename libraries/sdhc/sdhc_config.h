#ifndef SDHC_CONFIG_H
#define SDHC_CONFIG_H
/**
 * \addtogroup sdhc
 *
 * @{
 */
/**
 * \file
 * MMC/SD support configuration.
 */

/**
 * \ingroup sdhc_config
 * Controls MMC/SD write support.
 *
 * Set to 1 to enable MMC/SD write support, set to 0 to disable it.
 */
#define SDHC_WRITE_SUPPORT 1

/**
 * \ingroup sdhc_config
 * Controls MMC/SD write buffering.
 *
 * Set to 1 to buffer write accesses, set to 0 to disable it.
 *
 * \note This option has no effect when SDHC_WRITE_SUPPORT is 0.
 */
#define SDHC_WRITE_BUFFERING 1

/**
 * \ingroup sdhc_config
 * Controls MMC/SD access buffering.
 * 
 * Set to 1 to save static RAM, but be aware that you will
 * lose performance.
 *
 * \note When SDHC_WRITE_SUPPORT is 1, SDHC_SAVE_RAM will
 *       be reset to 0.
 */
#define SDHC_SAVE_RAM 1

/**
 * @}
 */

/* configuration checks */
#if SDHC_WRITE_SUPPORT
#undef SDHC_SAVE_RAM
#define SDHC_SAVE_RAM 0
#else
#undef SDHC_WRITE_BUFFERING
#define SDHC_WRITE_BUFFERING 0
#endif

typedef uint64_t offset_t;

#endif

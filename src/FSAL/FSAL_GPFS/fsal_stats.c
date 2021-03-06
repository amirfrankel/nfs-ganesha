/*
 * vim:expandtab:shiftwidth=4:tabstop=4:
 */

/**
 *
 * \file    fsal_stats.c
 * \date    $Date: 2005/07/27 13:30:26 $
 * \version $Revision: 1.2 $
 * \brief   Statistics functions.
 *
 */
#include "config.h"

#include "fsal.h"
#include "fsal_internal.h"

/**
 * FSAL_get_stats:
 * Retrieve call statistics for current thread.
 *
 * \param stats (output):
 *        Pointer to the call statistics structure.
 * \param reset (input):
 *        Boolean that indicates if the stats must be reset.
 *
 * \return Nothing.
 */

void GPFSFSAL_get_stats(fsal_statistics_t * stats,  /* OUT */
                    bool reset        /* IN */
    )
{

  /* sanity check. */
  if(!stats)
    return;

  /* returns stats for this thread. */
  fsal_internal_getstats(stats);

  return;
}

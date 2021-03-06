/*
 * vim:expandtab:shiftwidth=8:tabstop=8:
 *
 * Copyright CEA/DAM/DIF  (2008)
 * contributeur : Philippe DENIEL   philippe.deniel@cea.fr
 *                Thomas LEIBOVICI  thomas.leibovici@cea.fr
 *
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public License
 * as published by the Free Software Foundation; either version 3 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA
 * 02110-1301 USA
 *
 * ---------------------------------------
 */

/**
 * @defgroup Cache_inode Cache Inode
 * @{
 */

/**
 * @file    cache_inode_get.c
 * @brief   Get and eventually cache an entry.
 *
 * Get and eventually cache an entry.
 *
 *
 */
#include "config.h"
#include "log.h"
#include "HashTable.h"
#include "fsal.h"
#include "cache_inode.h"
#include "cache_inode_hash.h"
#include "cache_inode_lru.h"

#include <unistd.h>
#include <sys/types.h>
#include <sys/param.h>
#include <time.h>
#include <pthread.h>
#include <assert.h>

/**
 *
 * @brief Gets an entry by using its fsdata as a key and caches it if needed.
 *
 * Gets an entry by using its fsdata as a key and caches it if needed.
 *
 * If a cache entry is returned, its refcount is incremented by one.
 *
 * It turns out we do need cache_inode_get_located functionality for
 * cases like lookupp on an entry returning itself when it isn't a
 * root.  Therefore, if the 'associated' parameter is equal to the got
 * cache entry, a reference count is incremented but the structure
 * pointed to by attr is NOT filled in.
 *
 * @param[in]  fsdata     File system data
 * @param[in]  associated Entry that may be equal to the got entry
 * @param[in]  req_ctx    Request context (user creds, client address etc)
 * @param[out] entry      The entry
 *
 * @return CACHE_INODE_SUCCESS or errors.
 */
cache_inode_status_t
cache_inode_get(cache_inode_fsal_data_t *fsdata,
		cache_entry_t *associated,
		const struct req_op_context *req_ctx,
		cache_entry_t **entry)
{
     fsal_status_t fsal_status = {0, 0};
     struct fsal_export *exp_hdl = NULL;
     struct fsal_obj_handle *new_hdl;
     cache_inode_status_t status = CACHE_INODE_SUCCESS;
     cih_latch_t latch;

     /* Do lookup */
     *entry = cih_get_by_fh_latched(&fsdata->fh_desc, &latch, CIH_GET_RLOCK);
     if (*entry) {
          /* take an extra reference within the critical section */
          if (cache_inode_lru_ref(*entry, LRU_REQ_INITIAL) !=
              CACHE_INODE_SUCCESS) {
              /* Dead entry.  Treat like a lookup failure. */
          } else {
              if (*entry == associated) {
                  /* Take a quick exit so we don't invert lock
                   * ordering. */
                  cih_latch_rele(&latch);
                  return (CACHE_INODE_SUCCESS);
              }
          }
     }
     cih_latch_rele(&latch);

     /* Cache miss, allocate a new entry */
     exp_hdl = fsdata->export;
     fsal_status = exp_hdl->ops->create_handle(exp_hdl, req_ctx,
                                               &fsdata->fh_desc,
                                               &new_hdl);
     if (FSAL_IS_ERROR( fsal_status )) {
         status = cache_inode_error_convert(fsal_status);
         LogDebug(COMPONENT_CACHE_INODE,
                  "could not get create_handle object");
         *entry = NULL;
         return status;
     }

     status = cache_inode_new_entry(new_hdl, CACHE_INODE_FLAG_NONE, entry);
     if (*entry == NULL) {
         return status;
     }

     status = CACHE_INODE_SUCCESS;

     /* This is the replacement for cache_inode_renew_entry.  Rather
        than calling that function at the start of every cache_inode
        call with the inode locked, we call cache_inode_check trust to
        perform 'heavyweight' (timed expiration of cached attributes,
        getattr-based directory trust) checks the first time after
        getting an inode.  It does all of the checks read-locked and
        only acquires a write lock if there's something requiring a
        change.

        There is a second light-weight check done before use of cached
        data that checks whether the bits saying that inode attributes
        or inode content are trustworthy have been cleared by, for
        example, FSAL_CB.

        To summarize, the current implementation is that policy-based
        trust of validity is checked once per logical series of
        operations at cache_inode_get, and asynchronous trust is
        checked with use (when the attributes are locked for reading,
        for example.) */

     if ((status = cache_inode_check_trust(*entry, req_ctx))
         != CACHE_INODE_SUCCESS) {
       goto out_put;
     }

     return status;

 out_put:
     cache_inode_put(*entry);
     *entry = NULL;
     return status;
} /* cache_inode_get */

/**
 *
 * @brief Release logical reference to a cache entry
 *
 * This function releases a logical reference to a cache entry
 * acquired by a previous call to cache_inode_get.
 *
 * The result is typically to decrement the reference count on entry,
 * but additional side effects include LRU adjustment, movement
 * to/from the protected LRU partition, or recyling if the caller has
 * raced an operation which made entry unreachable (and this current
 * caller has the last reference).  Caller MUST NOT make further
 * accesses to the memory pointed to by entry.
 *
 * @param[in] entry Cache entry being returned
 */
void cache_inode_put(cache_entry_t *entry)
{
  cache_inode_lru_unref(entry, LRU_FLAG_NONE);
}
/** @} */

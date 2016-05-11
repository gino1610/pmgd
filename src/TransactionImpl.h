#pragma once

#include <stddef.h>
#include <stdint.h>
#include <string.h>
#include <stack>
#include "TransactionManager.h"
#include "exception.h"
#include "transaction.h"
#include "callback.h"
#include "compiler.h"

namespace Jarvis {
    class GraphImpl;

    class TransactionImpl {
            struct JournalEntry;

            static THREAD TransactionImpl *_per_thread_tx;

            GraphImpl *_db;
            int _tx_type;
            bool _committed;

            TransactionHandle _tx_handle;
            JournalEntry *_jcur;

            TransactionImpl *_outer_tx;

            CallbackList<void *, TransactionImpl *> _commit_callback_list;

            void log_je(void *src, size_t len);
            void finalize_commit();
            static void rollback(const TransactionHandle &h,
                                 const JournalEntry *jend);

            TransactionId tx_id() const { return _tx_handle.id; }
            JournalEntry *jbegin()
                { return static_cast<JournalEntry *>(_tx_handle.jbegin); }
            JournalEntry *jend()
                { return static_cast<JournalEntry *>(_tx_handle.jend); }

        public:
            TransactionImpl(const TransactionImpl &) = delete;
            void operator=(const TransactionImpl &) = delete;
            TransactionImpl(GraphImpl *db, int options);
            ~TransactionImpl();

            GraphImpl *get_db() const { return _db; }

            bool is_read_write() const
                { return _tx_type & Transaction::ReadWrite; }

            void check_read_write()
            {
                if (!(_tx_type & Transaction::ReadWrite))
                    throw Exception(ReadOnly);
            }

            void register_commit_callback(void *key, std::function<void(TransactionImpl *)> f)
                { _commit_callback_list.register_callback(key, f); }

            std::function<void(TransactionImpl *)> *lookup_callback(void *key)
                { return _commit_callback_list.lookup_callback(key); }

            // log data; user performs the writes
            void log(void *ptr, size_t len);

            // log data; base to base+end
            template <typename T>
            void log_range(void *base, T *end)
                { log(base, (char *)(end + 1) - (char *)base); }

            // log old_val and write new_val
            template <typename T>
                void write(T *ptr, T new_val)
            {
                log(ptr, sizeof(T));
                *ptr = new_val;
            }

            // log dst and overwrite with src
            void write(void *dst, void *src, size_t len)
            {
                log(src, len);
                memcpy(dst, src, len);
            }

            // memset without logging
            // TBD: implement when there is a need.
            void memset_nolog(void *ptr, uint8_t val, size_t len);

            // write new_val without logging
            template <typename T>
            void write_nolog(T *ptr, T new_val)
            {
                *ptr = new_val;
                flush_range(ptr, sizeof *ptr);
            }

            // write without logging
            void write_nolog(void *dst, void *src, size_t len)
            {
                memcpy(dst, src, len);
                flush_range(dst, len);
            }

            void commit() { _committed = true; }

            // get current transaction
            static inline TransactionImpl *get_tx()
            {
                if (_per_thread_tx == NULL)
                    throw Exception(NoTransaction);
                return _per_thread_tx;
            }

            // flush a range using clflushopt. Caller must call
            // persistent_barrier to ensure the flushed data is durable.
            static void flush_range(void *ptr, size_t len);

            // roll-back the transaction
            static void recover_tx(const TransactionHandle &);
    };
};

//////////////////////////////////////////////////////////////////////////////
// Product: QP/C++, cooperative "Vanilla" kernel
// Last Updated for QP ver: 4.2.04 (modified to fit in one file)
// Date of the Last Update: Sep 25, 2011
//
//                    Q u a n t u m     L e a P s
//                    ---------------------------
//                    innovating embedded systems
//
// Copyright (C) 2002-2011 Quantum Leaps, LLC. All rights reserved.
//
// This software may be distributed and modified under the terms of the GNU
// General Public License version 2 (GPL) as published by the Free Software
// Foundation and appearing in the file GPL.TXT included in the packaging of
// this file. Please note that GPL Section 2[b] requires that all works based
// on this software must also be made publicly available under the terms of
// the GPL ("Copyleft").
//
// Alternatively, this software may be distributed and modified under the
// terms of Quantum Leaps commercial licenses, which expressly supersede
// the GPL and are specifically designed for licensees interested in
// retaining the proprietary status of their code.
//
// Contact information:
// Quantum Leaps Web site:  http://www.quantum-leaps.com
// e-mail:                  info@quantum-leaps.com
//////////////////////////////////////////////////////////////////////////////
#include "qp_port.h"                                                // QP port

#ifdef Q_USE_NAMESPACE
namespace QP {
#endif

Q_DEFINE_THIS_MODULE(qp)

// "qep_pkg.h" ===============================================================
/// internal QEP constants
enum QEPConst {
    QEP_EMPTY_SIG_ = 0,                ///< empty signal for internal use only

    /// maximum depth of state nesting (including the top level), must be >= 3
    QEP_MAX_NEST_DEPTH_ = 6
};

/// helper macro to trigger internal event in an HSM
#define QEP_TRIG_(state_, sig_) \
    ((*(state_))(this, &QEP_reservedEvt_[sig_]))

/// helper macro to trigger exit action in an HSM
#define QEP_EXIT_(state_) \
    if (QEP_TRIG_(state_, Q_EXIT_SIG) == Q_RET_HANDLED) { \
        QS_BEGIN_(QS_QEP_STATE_EXIT, QS::smObj_, this) \
            QS_OBJ_(this); \
            QS_FUN_(state_); \
        QS_END_() \
    }

/// helper macro to trigger entry action in an HSM
#define QEP_ENTER_(state_) \
    if (QEP_TRIG_(state_, Q_ENTRY_SIG) == Q_RET_HANDLED) { \
        QS_BEGIN_(QS_QEP_STATE_ENTRY, QS::smObj_, this) \
            QS_OBJ_(this); \
            QS_FUN_(state_); \
        QS_END_() \
    }

// "qep.cpp" =================================================================
// Package-scope objects -----------------------------------------------------
QEvent const QEP_reservedEvt_[] = {
#ifdef Q_EVT_CTOR
    (QSignal)QEP_EMPTY_SIG_,
    (QSignal)Q_ENTRY_SIG,
    (QSignal)Q_EXIT_SIG,
    (QSignal)Q_INIT_SIG
#else
    {(QSignal)QEP_EMPTY_SIG_, (uint8_t)0, (uint8_t)0},
    {(QSignal)Q_ENTRY_SIG,    (uint8_t)0, (uint8_t)0},
    {(QSignal)Q_EXIT_SIG,     (uint8_t)0, (uint8_t)0},
    {(QSignal)Q_INIT_SIG,     (uint8_t)0, (uint8_t)0}
#endif
};
//............................................................................
//lint -e970 -e971 -e778         ignore MISRA rules 13 and 14 in this function
char const Q_ROM * Q_ROM_VAR QEP::getVersion(void) {
    static char const Q_ROM Q_ROM_VAR version[] = {
        (char)(((QP_VERSION >> 12U) & 0xFU) + (uint8_t)'0'),
        '.',
        (char)(((QP_VERSION >>  8U) & 0xFU) + (uint8_t)'0'),
        '.',
        (char)(((QP_VERSION >>  4U) & 0xFU) + (uint8_t)'0'),
        (char)((QP_VERSION          & 0xFU) + (uint8_t)'0'),
        '\0'
    };
    return version;
}

// "qhsm_top.cpp" ============================================================
QState QHsm::top(QHsm *, QEvent const *) {
    return Q_IGNORED();                    // the top state ignores all events
}

// "qhsm_ini.cpp" ============================================================
QHsm::~QHsm() {
}
//............................................................................
void QHsm::init(QEvent const *e) {
    QStateHandler t;
    QS_INT_LOCK_KEY_

                              // the top-most initial transition must be taken
    Q_ALLEGE((*m_state)(this, e) == Q_RET_TRAN);

    t = (QStateHandler)&QHsm::top;              // HSM starts in the top state
    do {                                           // drill into the target...
        QStateHandler path[QEP_MAX_NEST_DEPTH_];
        int8_t ip = (int8_t)0;                  // transition entry path index


        QS_BEGIN_(QS_QEP_STATE_INIT, QS::smObj_, this)
            QS_OBJ_(this);                        // this state machine object
            QS_FUN_(t);                                    // the source state
            QS_FUN_(m_state);          // the target of the initial transition
        QS_END_()

        path[0] = m_state;
        (void)QEP_TRIG_(m_state, QEP_EMPTY_SIG_);
        while (m_state != t) {
            ++ip;
            path[ip] = m_state;
            (void)QEP_TRIG_(m_state, QEP_EMPTY_SIG_);
        }
        m_state = path[0];
                                               // entry path must not overflow
        Q_ASSERT(ip < (int8_t)QEP_MAX_NEST_DEPTH_);

        do {           // retrace the entry path in reverse (desired) order...
            QEP_ENTER_(path[ip]);                            // enter path[ip]
            --ip;
        } while (ip >= (int8_t)0);

        t = path[0];                   // current state becomes the new source
    } while (QEP_TRIG_(t, Q_INIT_SIG) == Q_RET_TRAN);
    m_state = t;

    QS_BEGIN_(QS_QEP_INIT_TRAN, QS::smObj_, this)
        QS_TIME_();                                              // time stamp
        QS_OBJ_(this);                            // this state machine object
        QS_FUN_(m_state);                              // the new active state
    QS_END_()
}

// "qhsm_dis.cpp" ============================================================
void QHsm::dispatch(QEvent const *e) {
    QStateHandler path[QEP_MAX_NEST_DEPTH_];
    QStateHandler s;
    QStateHandler t;
    QState r;
    QS_INT_LOCK_KEY_

    t = m_state;                                     // save the current state

    QS_BEGIN_(QS_QEP_DISPATCH, QS::smObj_, this)
        QS_TIME_();                                              // time stamp
        QS_SIG_(e->sig);                            // the signal of the event
        QS_OBJ_(this);                            // this state machine object
        QS_FUN_(t);                                       // the current state
    QS_END_()

    do {                                // process the event hierarchically...
        s = m_state;
        r = (*s)(this, e);                           // invoke state handler s
    } while (r == Q_RET_SUPER);

    if (r == Q_RET_TRAN) {                                // transition taken?
#ifdef Q_SPY
        QStateHandler src = s;       // save the transition source for tracing
#endif
        int8_t ip = (int8_t)(-1);               // transition entry path index
        int8_t iq;                       // helper transition entry path index

        path[0] = m_state;                // save the target of the transition
        path[1] = t;

        while (t != s) {       // exit current state to transition source s...
            if (QEP_TRIG_(t, Q_EXIT_SIG) == Q_RET_HANDLED) {   //exit handled?
                QS_BEGIN_(QS_QEP_STATE_EXIT, QS::smObj_, this)
                    QS_OBJ_(this);                // this state machine object
                    QS_FUN_(t);                            // the exited state
                QS_END_()

                (void)QEP_TRIG_(t, QEP_EMPTY_SIG_);    // find superstate of t
            }
            t = m_state;                       // m_state holds the superstate
        }

        t = path[0];                               // target of the transition

        if (s == t) {         // (a) check source==target (transition to self)
            QEP_EXIT_(s)                                    // exit the source
            ip = (int8_t)0;                                // enter the target
        }
        else {
            (void)QEP_TRIG_(t, QEP_EMPTY_SIG_);        // superstate of target
            t = m_state;
            if (s == t) {                   // (b) check source==target->super
                ip = (int8_t)0;                            // enter the target
            }
            else {
                (void)QEP_TRIG_(s, QEP_EMPTY_SIG_);       // superstate of src
                                     // (c) check source->super==target->super
                if (m_state == t) {
                    QEP_EXIT_(s)                            // exit the source
                    ip = (int8_t)0;                        // enter the target
                }
                else {
                                            // (d) check source->super==target
                    if (m_state == path[0]) {
                        QEP_EXIT_(s)                        // exit the source
                    }
                    else { // (e) check rest of source==target->super->super..
                           // and store the entry path along the way
                           //
                        iq = (int8_t)0;         // indicate that LCA not found
                        ip = (int8_t)1;     // enter target and its superstate
                        path[1] = t;          // save the superstate of target
                        t = m_state;                     // save source->super
                                                  // find target->super->super
                        r = QEP_TRIG_(path[1], QEP_EMPTY_SIG_);
                        while (r == Q_RET_SUPER) {
                            ++ip;
                            path[ip] = m_state;        // store the entry path
                            if (m_state == s) {           // is it the source?
                                iq = (int8_t)1;     // indicate that LCA found
                                               // entry path must not overflow
                                Q_ASSERT(ip < (int8_t)QEP_MAX_NEST_DEPTH_);
                                --ip;               // do not enter the source
                                r = Q_RET_HANDLED;       // terminate the loop
                            }
                            else {      // it is not the source, keep going up
                                r = QEP_TRIG_(m_state, QEP_EMPTY_SIG_);
                            }
                        }
                        if (iq == (int8_t)0) {       // the LCA not found yet?

                                               // entry path must not overflow
                            Q_ASSERT(ip < (int8_t)QEP_MAX_NEST_DEPTH_);

                            QEP_EXIT_(s)                   // exit the source

                            // (f) check the rest of source->super
                            //                  == target->super->super...
                            //
                            iq = ip;
                            r = Q_RET_IGNORED;       // indicate LCA NOT found
                            do {
                                if (t == path[iq]) {       // is this the LCA?
                                    r = Q_RET_HANDLED;   // indicate LCA found
                                    ip = (int8_t)(iq - 1); // do not enter LCA
                                    iq = (int8_t)(-1);   // terminate the loop
                                }
                                else {
                                    --iq;    // try lower superstate of target
                                }
                            } while (iq >= (int8_t)0);

                            if (r != Q_RET_HANDLED) {    // LCA not found yet?
                                // (g) check each source->super->...
                                // for each target->super...
                                //
                                r = Q_RET_IGNORED;             // keep looping
                                do {
                                                          // exit t unhandled?
                                    if (QEP_TRIG_(t, Q_EXIT_SIG)
                                        == Q_RET_HANDLED)
                                    {
                                        QS_BEGIN_(QS_QEP_STATE_EXIT,
                                                  QS::smObj_, this)
                                            QS_OBJ_(this);
                                            QS_FUN_(t);
                                        QS_END_()

                                        (void)QEP_TRIG_(t, QEP_EMPTY_SIG_);
                                    }
                                    t = m_state;         //  set to super of t
                                    iq = ip;
                                    do {
                                        if (t == path[iq]) {   // is this LCA?
                                                           // do not enter LCA
                                            ip = (int8_t)(iq - 1);
                                            iq = (int8_t)(-1);   //break inner
                                            r = Q_RET_HANDLED;   //break outer
                                        }
                                        else {
                                            --iq;
                                        }
                                    } while (iq >= (int8_t)0);
                                } while (r != Q_RET_HANDLED);
                            }
                        }
                    }
                }
            }
        }
                       // retrace the entry path in reverse (desired) order...
        for (; ip >= (int8_t)0; --ip) {
            QEP_ENTER_(path[ip])                             // enter path[ip]
        }
        t = path[0];                         // stick the target into register
        m_state = t;                               // update the current state

                                         // drill into the target hierarchy...
        while (QEP_TRIG_(t, Q_INIT_SIG) == Q_RET_TRAN) {

            QS_BEGIN_(QS_QEP_STATE_INIT, QS::smObj_, this)
                QS_OBJ_(this);                    // this state machine object
                QS_FUN_(t);                        // the source (pseudo)state
                QS_FUN_(m_state);              // the target of the transition
            QS_END_()

            ip = (int8_t)0;
            path[0] = m_state;
            (void)QEP_TRIG_(m_state, QEP_EMPTY_SIG_);       // find superstate
            while (m_state != t) {
                ++ip;
                path[ip] = m_state;
                (void)QEP_TRIG_(m_state, QEP_EMPTY_SIG_);   // find superstate
            }
            m_state = path[0];
                                               // entry path must not overflow
            Q_ASSERT(ip < (int8_t)QEP_MAX_NEST_DEPTH_);

            do {       // retrace the entry path in reverse (correct) order...
                QEP_ENTER_(path[ip])                         // enter path[ip]
                --ip;
            } while (ip >= (int8_t)0);

            t = path[0];
        }

        QS_BEGIN_(QS_QEP_TRAN, QS::smObj_, this)
            QS_TIME_();                                          // time stamp
            QS_SIG_(e->sig);                        // the signal of the event
            QS_OBJ_(this);                        // this state machine object
            QS_FUN_(src);                      // the source of the transition
            QS_FUN_(t);                                // the new active state
        QS_END_()

    }
    else {                                             // transition not taken
#ifdef Q_SPY
        if (r == Q_RET_IGNORED) {                            // event ignored?

            QS_BEGIN_(QS_QEP_IGNORED, QS::smObj_, this)
                QS_TIME_();                                      // time stamp
                QS_SIG_(e->sig);                    // the signal of the event
                QS_OBJ_(this);                    // this state machine object
                QS_FUN_(t);                               // the current state
            QS_END_()

        }
        else {                                                // event handled

            QS_BEGIN_(QS_QEP_INTERN_TRAN, QS::smObj_, this)
                QS_TIME_();                                      // time stamp
                QS_SIG_(e->sig);                    // the signal of the event
                QS_OBJ_(this);                    // this state machine object
                QS_FUN_(s);                // the state that handled the event
            QS_END_()

        }
#endif
    }
    m_state = t;                 // set new state or restore the current state
}

// "qf_pkg.h" ================================================================
                                    // QF-specific interrupt locking/unlocking
#ifndef QF_INT_KEY_TYPE

    /// \brief This is an internal macro for defining the interrupt lock key.
    ///
    /// The purpose of this macro is to enable writing the same code for the
    /// case when interrupt key is defined and when it is not. If the macro
    /// #QF_INT_KEY_TYPE is defined, this internal macro provides the
    /// definition of the lock key variable. Otherwise this macro is empty.
    /// \sa #QF_INT_KEY_TYPE
    #define QF_INT_LOCK_KEY_

    /// \brief This is an internal macro for locking interrupts.
    ///
    /// The purpose of this macro is to enable writing the same code for the
    /// case when interrupt key is defined and when it is not. If the macro
    /// #QF_INT_KEY_TYPE is defined, this internal macro invokes #QF_INT_LOCK
    /// passing the key variable as the parameter. Otherwise #QF_INT_LOCK
    /// is invoked with a dummy parameter.
    /// \sa #QF_INT_LOCK, #QK_INT_LOCK
    #define QF_INT_LOCK_()      QF_INT_LOCK(dummy)

    /// \brief This is an internal macro for unlocking interrupts.
    ///
    /// The purpose of this macro is to enable writing the same code for the
    /// case when interrupt key is defined and when it is not. If the macro
    /// #QF_INT_KEY_TYPE is defined, this internal macro invokes
    /// #QF_INT_UNLOCK passing the key variable as the parameter.
    /// Otherwise #QF_INT_UNLOCK is invoked with a dummy parameter.
    /// \sa #QF_INT_UNLOCK, #QK_INT_UNLOCK
    #define QF_INT_UNLOCK_()    QF_INT_UNLOCK(dummy)
#else
    #define QF_INT_LOCK_KEY_    QF_INT_KEY_TYPE intLockKey_;
    #define QF_INT_LOCK_()      QF_INT_LOCK(intLockKey_)
    #define QF_INT_UNLOCK_()    QF_INT_UNLOCK(intLockKey_)
#endif

// package-scope objects -----------------------------------------------------
extern QTimeEvt *QF_timeEvtListHead_;  ///< head of linked list of time events
extern QF_EPOOL_TYPE_ QF_pool_[QF_MAX_EPOOL];        ///< allocate event pools
extern uint8_t QF_maxPool_;                  ///< # of initialized event pools
extern QSubscrList *QF_subscrList_;             ///< the subscriber list array
extern QSignal QF_maxSignal_;                ///< the maximum published signal

//............................................................................
/// \brief Structure representing a free block in the Native QF Memory Pool
/// \sa ::QMPool
struct QFreeBlock {
    QFreeBlock *m_next;
};

/// \brief access to the poolId of an event \a e_
#define EVT_POOL_ID(e_)     ((e_)->poolId_)

/// \brief access to the refCtr of an event \a e_
#define EVT_REF_CTR(e_)     ((e_)->refCtr_)

/// \brief increment the refCtr of an event \a e_
#define EVT_INC_REF_CTR(e_) (++((QEvent *)(e_))->refCtr_)

/// \brief decrement the refCtr of an event \a e_
#define EVT_DEC_REF_CTR(e_) (--((QEvent *)(e_))->refCtr_)

// "qa_defer.cpp" ============================================================
//............................................................................
void QActive::defer(QEQueue *eq, QEvent const *e) {
    eq->postFIFO(e);
}
//............................................................................
uint8_t QActive::recall(QEQueue *eq) {
    QEvent const *e = eq->get();    // try to get an event from deferred queue
    if (e != (QEvent *)0) {                                // event available?

        postLIFO(e);      // post it to the front of the Active Object's queue

        QF_INT_LOCK_KEY_
        QF_INT_LOCK_();

        if (EVT_POOL_ID(e) != (uint8_t)0) {          // is it a dynamic event?

            // after posting to the AO's queue the event must be referenced
            // at least twice: once in the deferred event queue (eq->get()
            // did NOT decrement the reference counter) and once in the
            // AO's event queue.
            Q_ASSERT(EVT_REF_CTR(e) > (uint8_t)1);

            // we need to decrement the reference counter once, to account
            // for removing the event from the deferred event queue.
            //
            EVT_DEC_REF_CTR(e);             // decrement the reference counter
        }

        QF_INT_UNLOCK_();

        return (uint8_t)1;                                   // event recalled
    }
    else {
        return (uint8_t)0;                               // event not recalled
    }
}

// "qa_fifo.cpp" =============================================================
//............................................................................
#ifndef Q_SPY
void QActive::postFIFO(QEvent const *e) {
#else
void QActive::postFIFO(QEvent const *e, void const *sender) {
#endif

    QF_INT_LOCK_KEY_
    QF_INT_LOCK_();

    QS_BEGIN_NOLOCK_(QS_QF_ACTIVE_POST_FIFO, QS::aoObj_, this)
        QS_TIME_();                                               // timestamp
        QS_OBJ_(sender);                                  // the sender object
        QS_SIG_(e->sig);                            // the signal of the event
        QS_OBJ_(this);                                   // this active object
        QS_U8_(EVT_POOL_ID(e));                    // the pool Id of the event
        QS_U8_(EVT_REF_CTR(e));                  // the ref count of the event
        QS_EQC_(m_eQueue.m_nFree);                   // number of free entries
        QS_EQC_(m_eQueue.m_nMin);                // min number of free entries
    QS_END_NOLOCK_()

    if (EVT_POOL_ID(e) != (uint8_t)0) {              // is it a dynamic event?
        EVT_INC_REF_CTR(e);                 // increment the reference counter
    }

    if (m_eQueue.m_frontEvt == (QEvent *)0) {           // is the queue empty?
        m_eQueue.m_frontEvt = e;                     // deliver event directly
        QACTIVE_EQUEUE_SIGNAL_(this);                // signal the event queue
    }
    else {               // queue is not empty, leave event in the ring-buffer
                                        // queue must accept all posted events
        Q_ASSERT(m_eQueue.m_nFree != (QEQueueCtr)0);
        m_eQueue.m_ring[m_eQueue.m_head] = e;//insert e into the buffer (FIFO)
        if (m_eQueue.m_head == (QEQueueCtr)0) {      // need to wrap the head?
            m_eQueue.m_head = m_eQueue.m_end;                   // wrap around
        }
        --m_eQueue.m_head;

        --m_eQueue.m_nFree;                    // update number of free events
        if (m_eQueue.m_nMin > m_eQueue.m_nFree) {
            m_eQueue.m_nMin = m_eQueue.m_nFree;       // update minimum so far
        }
    }
    QF_INT_UNLOCK_();
}

// "qa_get_.cpp" =============================================================
QEvent const *QActive::get_(void) {
    QF_INT_LOCK_KEY_
    QF_INT_LOCK_();

    QACTIVE_EQUEUE_WAIT_(this);           // wait for event to arrive directly

    QEvent const *e = m_eQueue.m_frontEvt;

    if (m_eQueue.m_nFree != m_eQueue.m_end) { //any events in the ring buffer?
                                                 // remove event from the tail
        m_eQueue.m_frontEvt = m_eQueue.m_ring[m_eQueue.m_tail];
        if (m_eQueue.m_tail == (QEQueueCtr)0) {      // need to wrap the tail?
            m_eQueue.m_tail = m_eQueue.m_end;                   // wrap around
        }
        --m_eQueue.m_tail;

        ++m_eQueue.m_nFree;          // one more free event in the ring buffer

        QS_BEGIN_NOLOCK_(QS_QF_ACTIVE_GET, QS::aoObj_, this)
            QS_TIME_();                                           // timestamp
            QS_SIG_(e->sig);                       // the signal of this event
            QS_OBJ_(this);                               // this active object
            QS_U8_(EVT_POOL_ID(e));                // the pool Id of the event
            QS_U8_(EVT_REF_CTR(e));              // the ref count of the event
            QS_EQC_(m_eQueue.m_nFree);               // number of free entries
        QS_END_NOLOCK_()
    }
    else {
        m_eQueue.m_frontEvt = (QEvent *)0;          // the queue becomes empty
        QACTIVE_EQUEUE_ONEMPTY_(this);

        QS_BEGIN_NOLOCK_(QS_QF_ACTIVE_GET_LAST, QS::aoObj_, this)
            QS_TIME_();                                           // timestamp
            QS_SIG_(e->sig);                       // the signal of this event
            QS_OBJ_(this);                               // this active object
            QS_U8_(EVT_POOL_ID(e));                // the pool Id of the event
            QS_U8_(EVT_REF_CTR(e));              // the ref count of the event
        QS_END_NOLOCK_()
    }
    QF_INT_UNLOCK_();
    return e;
}
//............................................................................
uint32_t QF::getQueueMargin(uint8_t prio) {
    Q_REQUIRE((prio <= (uint8_t)QF_MAX_ACTIVE)
              && (active_[prio] != (QActive *)0));

    QF_INT_LOCK_KEY_
    QF_INT_LOCK_();
    uint32_t margin = (uint32_t)(active_[prio]->m_eQueue.m_nMin);
    QF_INT_UNLOCK_();

    return margin;
}

// "qa_lifo.cpp" =============================================================
void QActive::postLIFO(QEvent const *e) {
    QF_INT_LOCK_KEY_
    QF_INT_LOCK_();

    QS_BEGIN_NOLOCK_(QS_QF_ACTIVE_POST_LIFO, QS::aoObj_, this)
        QS_TIME_();                                               // timestamp
        QS_SIG_(e->sig);                           // the signal of this event
        QS_OBJ_(this);                                   // this active object
        QS_U8_(EVT_POOL_ID(e));                    // the pool Id of the event
        QS_U8_(EVT_REF_CTR(e));                  // the ref count of the event
        QS_EQC_(m_eQueue.m_nFree);                   // number of free entries
        QS_EQC_(m_eQueue.m_nMin);                // min number of free entries
    QS_END_NOLOCK_()

    if (EVT_POOL_ID(e) != (uint8_t)0) {              // is it a dynamic event?
        EVT_INC_REF_CTR(e);                 // increment the reference counter
    }

    if (m_eQueue.m_frontEvt == (QEvent *)0) {           // is the queue empty?
        m_eQueue.m_frontEvt = e;                     // deliver event directly
        QACTIVE_EQUEUE_SIGNAL_(this);                // signal the event queue
    }
    else {               // queue is not empty, leave event in the ring-buffer
                                        // queue must accept all posted events
        Q_ASSERT(m_eQueue.m_nFree != (QEQueueCtr)0);

        ++m_eQueue.m_tail;
        if (m_eQueue.m_tail == m_eQueue.m_end) {     // need to wrap the tail?
            m_eQueue.m_tail = (QEQueueCtr)0;                    // wrap around
        }

        m_eQueue.m_ring[m_eQueue.m_tail] = m_eQueue.m_frontEvt;
        m_eQueue.m_frontEvt = e;                         // put event to front

        --m_eQueue.m_nFree;                    // update number of free events
        if (m_eQueue.m_nMin > m_eQueue.m_nFree) {
            m_eQueue.m_nMin = m_eQueue.m_nFree;       // update minimum so far
        }
    }
    QF_INT_UNLOCK_();
}

// "qa_sub.cpp" ==============================================================
void QActive::subscribe(QSignal sig) const {
    uint8_t p = m_prio;
    Q_REQUIRE(((QSignal)Q_USER_SIG <= sig)
              && (sig < QF_maxSignal_)
              && ((uint8_t)0 < p) && (p <= (uint8_t)QF_MAX_ACTIVE)
              && (QF::active_[p] == this));

    uint8_t i = Q_ROM_BYTE(QF_div8Lkup[p]);

    QF_INT_LOCK_KEY_
    QF_INT_LOCK_();

    QS_BEGIN_NOLOCK_(QS_QF_ACTIVE_SUBSCRIBE, QS::aoObj_, this)
        QS_TIME_();                                               // timestamp
        QS_SIG_(sig);                              // the signal of this event
        QS_OBJ_(this);                                   // this active object
    QS_END_NOLOCK_()
                                                       // set the priority bit
    QF_subscrList_[sig].m_bits[i] |= Q_ROM_BYTE(QF_pwr2Lkup[p]);
    QF_INT_UNLOCK_();
}

// "qa_usub.cpp" =============================================================
void QActive::unsubscribe(QSignal sig) const {
    uint8_t p = m_prio;
    Q_REQUIRE(((QSignal)Q_USER_SIG <= sig)
              && (sig < QF_maxSignal_)
              && ((uint8_t)0 < p) && (p <= (uint8_t)QF_MAX_ACTIVE)
              && (QF::active_[p] == this));

    uint8_t i = Q_ROM_BYTE(QF_div8Lkup[p]);

    QF_INT_LOCK_KEY_
    QF_INT_LOCK_();

    QS_BEGIN_NOLOCK_(QS_QF_ACTIVE_UNSUBSCRIBE, QS::aoObj_, this)
        QS_TIME_();                                               // timestamp
        QS_SIG_(sig);                              // the signal of this event
        QS_OBJ_(this);                                   // this active object
    QS_END_NOLOCK_()
                                                     // clear the priority bit
    QF_subscrList_[sig].m_bits[i] &= Q_ROM_BYTE(QF_invPwr2Lkup[p]);
    QF_INT_UNLOCK_();
}

// "qa_usuba.cpp" ============================================================
void QActive::unsubscribeAll(void) const {
    uint8_t p = m_prio;
    Q_REQUIRE(((uint8_t)0 < p) && (p <= (uint8_t)QF_MAX_ACTIVE)
              && (QF::active_[p] == this));

    uint8_t i = Q_ROM_BYTE(QF_div8Lkup[p]);

    QSignal sig;
    for (sig = (QSignal)Q_USER_SIG; sig < QF_maxSignal_; ++sig) {
        QF_INT_LOCK_KEY_
        QF_INT_LOCK_();
        if ((QF_subscrList_[sig].m_bits[i] & Q_ROM_BYTE(QF_pwr2Lkup[p]))
             != 0)
        {

            QS_BEGIN_NOLOCK_(QS_QF_ACTIVE_UNSUBSCRIBE, QS::aoObj_, this)
                QS_TIME_();                                       // timestamp
                QS_SIG_(sig);                      // the signal of this event
                QS_OBJ_(this);                           // this active object
            QS_END_NOLOCK_()
                                                     // clear the priority bit
            QF_subscrList_[sig].m_bits[i] &= Q_ROM_BYTE(QF_invPwr2Lkup[p]);
        }
        QF_INT_UNLOCK_();
    }
}

// "qeq_fifo.cpp" ============================================================
void QEQueue::postFIFO(QEvent const *e) {
    QF_INT_LOCK_KEY_
    QF_INT_LOCK_();

    QS_BEGIN_NOLOCK_(QS_QF_EQUEUE_POST_FIFO, QS::eqObj_, this)
        QS_TIME_();                                               // timestamp
        QS_SIG_(e->sig);                           // the signal of this event
        QS_OBJ_(this);                                    // this queue object
        QS_U8_(EVT_POOL_ID(e));                    // the pool Id of the event
        QS_U8_(EVT_REF_CTR(e));                  // the ref count of the event
        QS_EQC_(m_nFree);                            // number of free entries
        QS_EQC_(m_nMin);                         // min number of free entries
    QS_END_NOLOCK_()

    if (EVT_POOL_ID(e) != (uint8_t)0) {              // is it a dynamic event?
        EVT_INC_REF_CTR(e);                 // increment the reference counter
    }

    if (m_frontEvt == (QEvent *)0) {                    // is the queue empty?
        m_frontEvt = e;                              // deliver event directly
    }
    else {               // queue is not empty, leave event in the ring-buffer
               // the queue must be able to accept the event (cannot overflow)
        Q_ASSERT(m_nFree != (QEQueueCtr)0);

        m_ring[m_head] = e;             // insert event into the buffer (FIFO)
        if (m_head == (QEQueueCtr)0) {               // need to wrap the head?
            m_head = m_end;                                     // wrap around
        }
        --m_head;

        --m_nFree;                             // update number of free events
        if (m_nMin > m_nFree) {
            m_nMin = m_nFree;                         // update minimum so far
        }
    }
    QF_INT_UNLOCK_();
}

// "qeq_get.cpp" =============================================================
QEvent const *QEQueue::get(void) {
    QEvent const *e;
    QF_INT_LOCK_KEY_

    QF_INT_LOCK_();
    if (m_frontEvt == (QEvent *)0) {                    // is the queue empty?
        e = (QEvent *)0;                    // no event available at this time
    }
    else {
        e = m_frontEvt;

        if (m_nFree != m_end) {          // any events in the the ring buffer?
            m_frontEvt = m_ring[m_tail];         // remove event from the tail
            if (m_tail == (QEQueueCtr)0) {           // need to wrap the tail?
                m_tail = m_end;                                 // wrap around
            }
            --m_tail;

            ++m_nFree;               // one more free event in the ring buffer

            QS_BEGIN_NOLOCK_(QS_QF_EQUEUE_GET, QS::eqObj_, this)
                QS_TIME_();                                       // timestamp
                QS_SIG_(e->sig);                   // the signal of this event
                QS_OBJ_(this);                            // this queue object
                QS_U8_(EVT_POOL_ID(e));            // the pool Id of the event
                QS_U8_(EVT_REF_CTR(e));          // the ref count of the event
                QS_EQC_(m_nFree);                    // number of free entries
            QS_END_NOLOCK_()
        }
        else {
            m_frontEvt = (QEvent *)0;               // the queue becomes empty

            QS_BEGIN_NOLOCK_(QS_QF_EQUEUE_GET_LAST, QS::eqObj_, this)
                QS_TIME_();                                       // timestamp
                QS_SIG_(e->sig);                   // the signal of this event
                QS_OBJ_(this);                            // this queue object
                QS_U8_(EVT_POOL_ID(e));            // the pool Id of the event
                QS_U8_(EVT_REF_CTR(e));          // the ref count of the event
            QS_END_NOLOCK_()
        }
    }
    QF_INT_UNLOCK_();
    return e;
}

// "qeq_init.cpp" ============================================================
void QEQueue::init(QEvent const *qSto[], QEQueueCtr qLen) {
    m_frontEvt = (QEvent *)0;                        // no events in the queue
    m_ring     = &qSto[0];
    m_end      = qLen;
    m_head     = (QEQueueCtr)0;
    m_tail     = (QEQueueCtr)0;
    m_nFree    = qLen;
    m_nMin     = qLen;

    QS_INT_LOCK_KEY_
    QS_BEGIN_(QS_QF_EQUEUE_INIT, QS::eqObj_, this)
        QS_OBJ_(qSto);                                  // this QEQueue object
        QS_EQC_(qLen);                              // the length of the queue
    QS_END_()
}

// "qeq_lifo.cpp" ============================================================
void QEQueue::postLIFO(QEvent const *e) {
    QF_INT_LOCK_KEY_
    QF_INT_LOCK_();

    QS_BEGIN_NOLOCK_(QS_QF_EQUEUE_POST_LIFO, QS::eqObj_, this)
        QS_TIME_();                                               // timestamp
        QS_SIG_(e->sig);                           // the signal of this event
        QS_OBJ_(this);                                    // this queue object
        QS_U8_(EVT_POOL_ID(e));                    // the pool Id of the event
        QS_U8_(EVT_REF_CTR(e));                  // the ref count of the event
        QS_EQC_(m_nFree);                            // number of free entries
        QS_EQC_(m_nMin);                         // min number of free entries
    QS_END_NOLOCK_()

    if (EVT_POOL_ID(e) != (uint8_t)0) {              // is it a dynamic event?
        EVT_INC_REF_CTR(e);                 // increment the reference counter
    }

    if (m_frontEvt != (QEvent *)0) {                // is the queue not empty?
               // the queue must be able to accept the event (cannot overflow)
        Q_ASSERT(m_nFree != (QEQueueCtr)0);

        ++m_tail;
        if (m_tail == m_end) {                       // need to wrap the tail?
            m_tail = (QEQueueCtr)0;                             // wrap around
        }

        m_ring[m_tail] = m_frontEvt;               // buffer the old front evt

        --m_nFree;                             // update number of free events
        if (m_nMin > m_nFree) {
            m_nMin = m_nFree;                         // update minimum so far
        }
    }

    m_frontEvt = e;                        // stick the new event to the front

    QF_INT_UNLOCK_();
}

// "qf_act.cpp" ==============================================================
// public objects ------------------------------------------------------------
QActive *QF::active_[QF_MAX_ACTIVE + 1];        // to be used by QF ports only
uint8_t QF_intLockNest_;                       // interrupt-lock nesting level

//............................................................................
//lint -e970 -e971               ignore MISRA rules 13 and 14 in this function
const char Q_ROM * Q_ROM_VAR QF::getVersion(void) {
    static char const Q_ROM Q_ROM_VAR version[] = {
        (char)(((QP_VERSION >> 12U) & 0xFU) + (uint8_t)'0'),
        '.',
        (char)(((QP_VERSION >>  8U) & 0xFU) + (uint8_t)'0'),
        '.',
        (char)(((QP_VERSION >>  4U) & 0xFU) + (uint8_t)'0'),
        (char)((QP_VERSION          & 0xFU) + (uint8_t)'0'),
        '\0'
    };
    return version;
}
//............................................................................
void QF::add_(QActive *a) {
    uint8_t p = a->m_prio;

    Q_REQUIRE(((uint8_t)0 < p) && (p <= (uint8_t)QF_MAX_ACTIVE)
              && (active_[p] == (QActive *)0));

    QF_INT_LOCK_KEY_
    QF_INT_LOCK_();

    active_[p] = a;            // registger the active object at this priority

    QS_BEGIN_NOLOCK_(QS_QF_ACTIVE_ADD, QS::aoObj_, a)
        QS_TIME_();                                               // timestamp
        QS_OBJ_(a);                                       // the active object
        QS_U8_(p);                        // the priority of the active object
    QS_END_NOLOCK_()

    QF_INT_UNLOCK_();
}
//............................................................................
void QF::remove_(QActive const *a) {
    uint8_t p = a->m_prio;

    Q_REQUIRE(((uint8_t)0 < p) && (p <= (uint8_t)QF_MAX_ACTIVE)
              && (active_[p] == a));

    QF_INT_LOCK_KEY_
    QF_INT_LOCK_();

    active_[p] = (QActive *)0;                   // free-up the priority level

    QS_BEGIN_NOLOCK_(QS_QF_ACTIVE_REMOVE, QS::aoObj_, a)
        QS_TIME_();                                               // timestamp
        QS_OBJ_(a);                                       // the active object
        QS_U8_(p);                        // the priority of the active object
    QS_END_NOLOCK_()

    QF_INT_UNLOCK_();
}

// "qf_gc.cpp" ===============================================================
void QF::gc(QEvent const *e) {
    if (EVT_POOL_ID(e) != (uint8_t)0) {              // is it a dynamic event?
        QF_INT_LOCK_KEY_
        QF_INT_LOCK_();

        if (EVT_REF_CTR(e) > (uint8_t)1) {   // isn't this the last reference?
            EVT_DEC_REF_CTR(e);                   // decrement the ref counter

            QS_BEGIN_NOLOCK_(QS_QF_GC_ATTEMPT, (void *)0, (void *)0)
                QS_TIME_();                                       // timestamp
                QS_SIG_(e->sig);                    // the signal of the event
                QS_U8_(EVT_POOL_ID(e));            // the pool Id of the event
                QS_U8_(EVT_REF_CTR(e));          // the ref count of the event
            QS_END_NOLOCK_()

            QF_INT_UNLOCK_();
        }
        else {         // this is the last reference to this event, recycle it
            uint8_t idx = (uint8_t)(EVT_POOL_ID(e) - 1);

            QS_BEGIN_NOLOCK_(QS_QF_GC, (void *)0, (void *)0)
                QS_TIME_();                                       // timestamp
                QS_SIG_(e->sig);                    // the signal of the event
                QS_U8_(EVT_POOL_ID(e));            // the pool Id of the event
                QS_U8_(EVT_REF_CTR(e));          // the ref count of the event
            QS_END_NOLOCK_()

            QF_INT_UNLOCK_();

            Q_ASSERT(idx < QF_maxPool_);

#ifdef Q_EVT_CTOR
            //lint -e1773                           Attempt to cast away const
            ((QEvent *)e)->~QEvent();     // call the xtor, cast 'const' away,
                             // which is legitimate, because it's a pool event
#endif
            //lint -e1773                           Attempt to cast away const
            QF_EPOOL_PUT_(QF_pool_[idx], (QEvent *)e);   // cast 'const' away,
                             // which is legitimate, because it's a pool event
        }
    }
}

// "qf_log2.cpp" =============================================================
uint8_t const Q_ROM Q_ROM_VAR QF_log2Lkup[256] = {
    0U, 1U, 2U, 2U, 3U, 3U, 3U, 3U, 4U, 4U, 4U, 4U, 4U, 4U, 4U, 4U,
    5U, 5U, 5U, 5U, 5U, 5U, 5U, 5U, 5U, 5U, 5U, 5U, 5U, 5U, 5U, 5U,
    6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U,
    6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U,
    7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U,
    7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U,
    7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U,
    7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U,
    8U, 8U, 8U, 8U, 8U, 8U, 8U, 8U, 8U, 8U, 8U, 8U, 8U, 8U, 8U, 8U,
    8U, 8U, 8U, 8U, 8U, 8U, 8U, 8U, 8U, 8U, 8U, 8U, 8U, 8U, 8U, 8U,
    8U, 8U, 8U, 8U, 8U, 8U, 8U, 8U, 8U, 8U, 8U, 8U, 8U, 8U, 8U, 8U,
    8U, 8U, 8U, 8U, 8U, 8U, 8U, 8U, 8U, 8U, 8U, 8U, 8U, 8U, 8U, 8U,
    8U, 8U, 8U, 8U, 8U, 8U, 8U, 8U, 8U, 8U, 8U, 8U, 8U, 8U, 8U, 8U,
    8U, 8U, 8U, 8U, 8U, 8U, 8U, 8U, 8U, 8U, 8U, 8U, 8U, 8U, 8U, 8U,
    8U, 8U, 8U, 8U, 8U, 8U, 8U, 8U, 8U, 8U, 8U, 8U, 8U, 8U, 8U, 8U,
    8U, 8U, 8U, 8U, 8U, 8U, 8U, 8U, 8U, 8U, 8U, 8U, 8U, 8U, 8U, 8U
};

// "qf_new.cpp" ==============================================================
QEvent *QF::new_(uint16_t evtSize, QSignal sig) {
                    // find the pool id that fits the requested event size ...
    uint8_t idx = (uint8_t)0;
    while (evtSize > QF_EPOOL_EVENT_SIZE_(QF_pool_[idx])) {
        ++idx;
        Q_ASSERT(idx < QF_maxPool_);     // cannot run out of registered pools
    }

    QS_INT_LOCK_KEY_
    QS_BEGIN_(QS_QF_NEW, (void *)0, (void *)0)
        QS_TIME_();                                               // timestamp
        QS_EVS_(evtSize);                             // the size of the event
        QS_SIG_(sig);                               // the signal of the event
    QS_END_()

    QEvent *e;
    QF_EPOOL_GET_(QF_pool_[idx], e);
    Q_ASSERT(e != (QEvent *)0);             // pool must not run out of events

    e->sig = sig;                                 // set signal for this event
    EVT_POOL_ID(e) = (uint8_t)(idx + 1);     // store the pool ID in the event
    EVT_REF_CTR(e) = (uint8_t)0;             // set the reference counter to 0

    return e;
}

// "qf_pool.cpp" =============================================================
// Package-scope objects -----------------------------------------------------
QF_EPOOL_TYPE_ QF_pool_[QF_MAX_EPOOL];             // allocate the event pools
uint8_t QF_maxPool_;                      // number of initialized event pools

//............................................................................
void QF::poolInit(void *poolSto, uint32_t poolSize, QEventSize evtSize) {
                         // cannot exceed the number of available memory pools
    Q_REQUIRE(QF_maxPool_ < (uint8_t)Q_DIM(QF_pool_));
               // please initialize event pools in ascending order of evtSize:
    Q_REQUIRE((QF_maxPool_ == (uint8_t)0)
              || (QF_EPOOL_EVENT_SIZE_(QF_pool_[QF_maxPool_ - 1]) < evtSize));
    QF_EPOOL_INIT_(QF_pool_[QF_maxPool_], poolSto, poolSize, evtSize);
    ++QF_maxPool_;                                            // one more pool
}

// "qf_psini.cpp" ============================================================
// Package-scope objects -----------------------------------------------------
QSubscrList *QF_subscrList_;
QSignal QF_maxSignal_;

//............................................................................
void QF::psInit(QSubscrList *subscrSto, QSignal maxSignal) {
    QF_subscrList_ = subscrSto;
    QF_maxSignal_ = maxSignal;
}

// "qf_pspub.cpp" ============================================================
#ifndef Q_SPY
void QF::publish(QEvent const *e) {
#else
void QF::publish(QEvent const *e, void const *sender) {
#endif
         // make sure that the published signal is within the configured range
    Q_REQUIRE(e->sig < QF_maxSignal_);

    QF_INT_LOCK_KEY_
    QF_INT_LOCK_();

    QS_BEGIN_NOLOCK_(QS_QF_PUBLISH, (void *)0, (void *)0)
        QS_TIME_();                                           // the timestamp
        QS_OBJ_(sender);                                  // the sender object
        QS_SIG_(e->sig);                            // the signal of the event
        QS_U8_(EVT_POOL_ID(e));                    // the pool Id of the event
        QS_U8_(EVT_REF_CTR(e));                  // the ref count of the event
    QS_END_NOLOCK_()

    if (EVT_POOL_ID(e) != (uint8_t)0) {              // is it a dynamic event?
        EVT_INC_REF_CTR(e);         // increment the reference counter, NOTE01
    }
    QF_INT_UNLOCK_();

#if (QF_MAX_ACTIVE <= 8)
    uint8_t tmp = QF_subscrList_[e->sig].m_bits[0];
    while (tmp != (uint8_t)0) {
        uint8_t p = Q_ROM_BYTE(QF_log2Lkup[tmp]);
        tmp &= Q_ROM_BYTE(QF_invPwr2Lkup[p]);      // clear the subscriber bit
        Q_ASSERT(active_[p] != (QActive *)0);            // must be registered

                           // POST() asserts internally if the queue overflows
        active_[p]->POST(e, sender);
    }
#else
    uint8_t i = Q_DIM(QF_subscrList_[0].m_bits);// number of bytes in the list
    do {                       // go through all bytes in the subsciption list
        --i;
        uint8_t tmp = QF_subscrList_[e->sig].m_bits[i];
        while (tmp != (uint8_t)0) {
            uint8_t p = Q_ROM_BYTE(QF_log2Lkup[tmp]);
            tmp &= Q_ROM_BYTE(QF_invPwr2Lkup[p]);  // clear the subscriber bit
            p = (uint8_t)(p + (i << 3));                // adjust the priority
            Q_ASSERT(active_[p] != (QActive *)0);        // must be registered

                           // POST() asserts internally if the queue overflows
            active_[p]->POST(e, sender);
        }
    } while (i != (uint8_t)0);
#endif

    gc(e);                            // run the garbage collector, see NOTE01
}

// "qf_pwr2.cpp" =============================================================
// Global objects ------------------------------------------------------------
uint8_t const Q_ROM Q_ROM_VAR QF_pwr2Lkup[65] = {
    0x00U,                                                  // unused location
    0x01U, 0x02U, 0x04U, 0x08U, 0x10U, 0x20U, 0x40U, 0x80U,
    0x01U, 0x02U, 0x04U, 0x08U, 0x10U, 0x20U, 0x40U, 0x80U,
    0x01U, 0x02U, 0x04U, 0x08U, 0x10U, 0x20U, 0x40U, 0x80U,
    0x01U, 0x02U, 0x04U, 0x08U, 0x10U, 0x20U, 0x40U, 0x80U,
    0x01U, 0x02U, 0x04U, 0x08U, 0x10U, 0x20U, 0x40U, 0x80U,
    0x01U, 0x02U, 0x04U, 0x08U, 0x10U, 0x20U, 0x40U, 0x80U,
    0x01U, 0x02U, 0x04U, 0x08U, 0x10U, 0x20U, 0x40U, 0x80U,
    0x01U, 0x02U, 0x04U, 0x08U, 0x10U, 0x20U, 0x40U, 0x80U
};

uint8_t const Q_ROM Q_ROM_VAR QF_invPwr2Lkup[65] = {
    0xFFU,                                                  // unused location
    0xFEU, 0xFDU, 0xFBU, 0xF7U, 0xEFU, 0xDFU, 0xBFU, 0x7FU,
    0xFEU, 0xFDU, 0xFBU, 0xF7U, 0xEFU, 0xDFU, 0xBFU, 0x7FU,
    0xFEU, 0xFDU, 0xFBU, 0xF7U, 0xEFU, 0xDFU, 0xBFU, 0x7FU,
    0xFEU, 0xFDU, 0xFBU, 0xF7U, 0xEFU, 0xDFU, 0xBFU, 0x7FU,
    0xFEU, 0xFDU, 0xFBU, 0xF7U, 0xEFU, 0xDFU, 0xBFU, 0x7FU,
    0xFEU, 0xFDU, 0xFBU, 0xF7U, 0xEFU, 0xDFU, 0xBFU, 0x7FU,
    0xFEU, 0xFDU, 0xFBU, 0xF7U, 0xEFU, 0xDFU, 0xBFU, 0x7FU,
    0xFEU, 0xFDU, 0xFBU, 0xF7U, 0xEFU, 0xDFU, 0xBFU, 0x7FU
};

uint8_t const Q_ROM Q_ROM_VAR QF_div8Lkup[65] = {
    0U,                                                     // unused location
    0U, 0U, 0U, 0U, 0U, 0U, 0U, 0U,
    1U, 1U, 1U, 1U, 1U, 1U, 1U, 1U,
    2U, 2U, 2U, 2U, 2U, 2U, 2U, 2U,
    3U, 3U, 3U, 3U, 3U, 3U, 3U, 3U,
    4U, 4U, 4U, 4U, 4U, 4U, 4U, 4U,
    5U, 5U, 5U, 5U, 5U, 5U, 5U, 5U,
    6U, 6U, 6U, 6U, 6U, 6U, 6U, 6U,
    7U, 7U, 7U, 7U, 7U, 7U, 7U, 7U
};

// "qf_tick.cpp" =============================================================
#ifndef Q_SPY
void QF::tick(void) {                                            // see NOTE01
#else
void QF::tick(void const *sender) {
#endif

    QF_INT_LOCK_KEY_
    QF_INT_LOCK_();

    QS_BEGIN_NOLOCK_(QS_QF_TICK, (void *)0, (void *)0)
        QS_TEC_(++QS::tickCtr_);                           // the tick counter
    QS_END_NOLOCK_()

    QTimeEvt *t = QF_timeEvtListHead_;
    while (t != (QTimeEvt *)0) {
        --t->m_ctr;
        if (t->m_ctr == (QTimeEvtCtr)0) {  // is the time evt about to expire?
            if (t->m_interval != (QTimeEvtCtr)0) {//is it a periodic time evt?
                t->m_ctr = t->m_interval;                // rearm the time evt
            }
            else {   // one-shot time evt, disarm by removing it from the list
                if (t == QF_timeEvtListHead_) {
                    QF_timeEvtListHead_ = t->m_next;
                }
                else {
                    if (t->m_next != (QTimeEvt *)0) {// not the last time evt?
                        t->m_next->m_prev = t->m_prev;
                    }
                    t->m_prev->m_next = t->m_next;
                }
                t->m_prev = (QTimeEvt *)0;     // mark the time event disarmed

                QS_BEGIN_NOLOCK_(QS_QF_TIMEEVT_AUTO_DISARM, QS::teObj_, t)
                    QS_OBJ_(t);                      // this time event object
                    QS_OBJ_(t->m_act);                     // the active object
                QS_END_NOLOCK_()
            }

            QS_BEGIN_NOLOCK_(QS_QF_TIMEEVT_POST, QS::teObj_, t)
                QS_TIME_();                                       // timestamp
                QS_OBJ_(t);                           // the time event object
                QS_SIG_(t->sig);              // the signal of this time event
                QS_OBJ_(t->m_act);                        // the active object
            QS_END_NOLOCK_()

            QF_INT_UNLOCK_();   // unlock interrupts before calling QF service

                           // POST() asserts internally if the queue overflows
            t->m_act->POST(t, sender);
        }
        else {
            QF_INT_UNLOCK_();
            static uint8_t volatile dummy;
            dummy = (uint8_t)0;      // execute a few instructions, see NOTE02
        }

        QF_INT_LOCK_();           // lock interrupts again to advance the link
        t = t->m_next;
    }
    QF_INT_UNLOCK_();
}

// "qmp_get.cpp" =============================================================
void *QMPool::get(void) {
    QF_INT_LOCK_KEY_
    QF_INT_LOCK_();

    QFreeBlock *fb = (QFreeBlock *)m_free;         // get a free block or NULL
    if (fb != (QFreeBlock *)0) {                      // free block available?
        m_free = fb->m_next;        // adjust list head to the next free block
        --m_nFree;                                      // one free block less
        if (m_nMin > m_nFree) {
            m_nMin = m_nFree;                   // remember the minimum so far
        }
    }

    QS_BEGIN_NOLOCK_(QS_QF_MPOOL_GET, QS::mpObj_, m_start)
        QS_TIME_();                                               // timestamp
        QS_OBJ_(m_start);                   // the memory managed by this pool
        QS_MPC_(m_nFree);             // the number of free blocks in the pool
        QS_MPC_(m_nMin);     // the mninimum number of free blocks in the pool
    QS_END_NOLOCK_()

    QF_INT_UNLOCK_();
    return fb;               // return the block or NULL pointer to the caller
}
//............................................................................
uint32_t QF::getPoolMargin(uint8_t poolId) {
    Q_REQUIRE(((uint8_t)1 <= poolId) && (poolId <= QF_maxPool_));

    QF_INT_LOCK_KEY_
    QF_INT_LOCK_();
    uint32_t margin = (uint32_t)QF_pool_[poolId - (uint8_t)1].m_nMin;
    QF_INT_UNLOCK_();

    return margin;
}

// "qmp_init.cpp" ============================================================
void QMPool::init(void *poolSto, uint32_t poolSize, QMPoolSize blockSize) {
    // The memory block must be valid
    // and the poolSize must fit at least one free block
    // and the blockSize must not be too close to the top of the dynamic range
    Q_REQUIRE((poolSto != (void *)0)
              && (poolSize >= (uint32_t)sizeof(QFreeBlock))
              && ((QMPoolSize)(blockSize + (QMPoolSize)sizeof(QFreeBlock))
                    > blockSize));

    m_free = poolSto;

                // round up the blockSize to fit an integer number of pointers
    m_blockSize = (QMPoolSize)sizeof(QFreeBlock);       // start with just one
    uint32_t nblocks = (uint32_t)1;// # free blocks that fit in a memory block
    while (m_blockSize < blockSize) {
        m_blockSize += (QMPoolSize)sizeof(QFreeBlock);
        ++nblocks;
    }
    blockSize = m_blockSize;          // use the rounded-up value from here on

               // the whole pool buffer must fit at least one rounded-up block
    Q_ASSERT(poolSize >= (uint32_t)blockSize);

                                // chain all blocks together in a free-list...
    poolSize -= (uint32_t)blockSize;             // don't chain the last block
    m_nTot     = (QMPoolCtr)1;             // one (the last) block in the pool
    QFreeBlock *fb = (QFreeBlock *)m_free;//start at the head of the free list
    while (poolSize >= (uint32_t)blockSize) {
        fb->m_next = &fb[nblocks];                      // setup the next link
        fb = fb->m_next;                              // advance to next block
        poolSize -= (uint32_t)blockSize;     // reduce the available pool size
        ++m_nTot;                     // increment the number of blocks so far
    }

    fb->m_next = (QFreeBlock *)0;              // the last link points to NULL
    m_nFree    = m_nTot;                                // all blocks are free
    m_nMin     = m_nTot;                  // the minimum number of free blocks
    m_start    = poolSto;               // the original start this pool buffer
    m_end      = fb;                            // the last block in this pool

    QS_INT_LOCK_KEY_
    QS_BEGIN_(QS_QF_MPOOL_INIT, QS::mpObj_, m_start)
        QS_OBJ_(m_start);                   // the memory managed by this pool
        QS_MPC_(m_nTot);                         // the total number of blocks
    QS_END_()
}

// "qmp_put.cpp" =============================================================
void QMPool::put(void *b) {
    //lint -e946 -e1904             ignore MISRA Rule 103 in this precondition
    Q_REQUIRE((m_start <= b) && (b <= m_end)           /*  must be in range */
              && (m_nFree <= m_nTot));        // # free blocks must be < total

    QF_INT_LOCK_KEY_
    QF_INT_LOCK_();

    ((QFreeBlock *)b)->m_next = (QFreeBlock *)m_free;//link into the free list
    m_free = b;                            // set as new head of the free list
    ++m_nFree;                             // one more free block in this pool

    QS_BEGIN_NOLOCK_(QS_QF_MPOOL_PUT, QS::mpObj_, m_start)
        QS_TIME_();                                               // timestamp
        QS_OBJ_(m_start);                   // the memory managed by this pool
        QS_MPC_(m_nFree);             // the number of free blocks in the pool
    QS_END_NOLOCK_()

    QF_INT_UNLOCK_();
}

// "qte_arm.cpp" =============================================================
// Package-scope objects -----------------------------------------------------
QTimeEvt *QF_timeEvtListHead_;           // head of linked list of time events

//............................................................................
void QTimeEvt::arm_(QActive *act, QTimeEvtCtr nTicks) {
    Q_REQUIRE((nTicks > (QTimeEvtCtr)0)          /* cannot arm with 0 ticks */
              && (sig >= (QSignal)Q_USER_SIG)               /* valid signal */
              && (m_prev == (QTimeEvt *)0)   /* time event must NOT be used */
              && (act != (QActive *)0));  /* active object must be provided */
    m_ctr = nTicks;
    m_prev = this;                                    // mark the timer in use
    m_act = act;
    QF_INT_LOCK_KEY_
    QF_INT_LOCK_();

    QS_BEGIN_NOLOCK_(QS_QF_TIMEEVT_ARM, QS::teObj_, this)
        QS_TIME_();                                               // timestamp
        QS_OBJ_(this);                               // this time event object
        QS_OBJ_(act);                                     // the active object
        QS_TEC_(nTicks);                                // the number of ticks
        QS_TEC_(m_interval);                                   // the interval
    QS_END_NOLOCK_()

    m_next = QF_timeEvtListHead_;
    if (QF_timeEvtListHead_ != (QTimeEvt *)0) {
        QF_timeEvtListHead_->m_prev = this;
    }
    QF_timeEvtListHead_ = this;
    QF_INT_UNLOCK_();
}

// "qte_ctor.cpp" ============================================================
QTimeEvt::QTimeEvt(QSignal s)
  :
#ifdef Q_EVT_CTOR
    QEvent(s),
#endif
    m_prev((QTimeEvt *)0),
    m_next((QTimeEvt *)0),
    m_act((QActive *)0),
    m_ctr((QTimeEvtCtr)0),
    m_interval((QTimeEvtCtr)0)
{
    Q_REQUIRE(s >= (QSignal)Q_USER_SIG);                       // valid signal
    sig = s;
    EVT_POOL_ID(this) = (uint8_t)0;   // time event must be static, see NOTE01
}

// "qte_ctr.cpp" =============================================================
QTimeEvtCtr QTimeEvt::ctr(void) {
    QTimeEvtCtr ctr;
    QF_INT_LOCK_KEY_
    QF_INT_LOCK_();
    if (m_prev != (QTimeEvt *)0) {        // is the time event actually armed?
        ctr = m_ctr;
    }
    else {                                     // the time event was not armed
        ctr = (QTimeEvtCtr)0;
    }

    QS_BEGIN_NOLOCK_(QS_QF_TIMEEVT_CTR, QS::teObj_, this)
        QS_TIME_();                                               // timestamp
        QS_OBJ_(this);                               // this time event object
        QS_OBJ_(m_act);                                   // the active object
        QS_TEC_(ctr);                                   // the current counter
        QS_TEC_(m_interval);                                   // the interval
    QS_END_NOLOCK_()

    QF_INT_UNLOCK_();
    return ctr;
}

// "qte_darm.cpp" ============================================================
// NOTE: disarm a time evt (no harm in disarming an already disarmed time evt)
uint8_t QTimeEvt::disarm(void) {
    uint8_t wasArmed;
    QF_INT_LOCK_KEY_
    QF_INT_LOCK_();
    if (m_prev != (QTimeEvt *)0) {        // is the time event actually armed?
        wasArmed = (uint8_t)1;
        if (this == QF_timeEvtListHead_) {
            QF_timeEvtListHead_ = m_next;
        }
        else {
            if (m_next != (QTimeEvt *)0) {        // not the last in the list?
                m_next->m_prev = m_prev;
            }
            m_prev->m_next = m_next;
        }
        m_prev = (QTimeEvt *)0;             // mark the time event as disarmed

        QS_BEGIN_NOLOCK_(QS_QF_TIMEEVT_DISARM, QS::teObj_, this)
            QS_TIME_();                                           // timestamp
            QS_OBJ_(this);                           // this time event object
            QS_OBJ_(m_act);                               // the active object
            QS_TEC_(m_ctr);                             // the number of ticks
            QS_TEC_(m_interval);                               // the interval
        QS_END_NOLOCK_()
    }
    else {                                     // the time event was not armed
        wasArmed = (uint8_t)0;

        QS_BEGIN_NOLOCK_(QS_QF_TIMEEVT_DISARM_ATTEMPT, QS::teObj_, this)
            QS_TIME_();                                           // timestamp
            QS_OBJ_(this);                           // this time event object
            QS_OBJ_(m_act);                               // the active object
        QS_END_NOLOCK_()
    }
    QF_INT_UNLOCK_();
    return wasArmed;
}

// "qte_rarm.cpp" ============================================================
uint8_t QTimeEvt::rearm(QTimeEvtCtr nTicks) {
    Q_REQUIRE((nTicks > (QTimeEvtCtr)0)        /* cannot rearm with 0 ticks */
              && (sig >= (QSignal)Q_USER_SIG)               /* valid signal */
              && (m_act != (QActive *)0));              // valid active object
    uint8_t isArmed;
    QF_INT_LOCK_KEY_
    QF_INT_LOCK_();
    m_ctr = nTicks;
    if (m_prev == (QTimeEvt *)0) {             // is this time event disarmed?
        isArmed = (uint8_t)0;
        m_next = QF_timeEvtListHead_;
        if (QF_timeEvtListHead_ != (QTimeEvt *)0) {
            QF_timeEvtListHead_->m_prev = this;
        }
        QF_timeEvtListHead_ = this;
        m_prev = this;                             // mark the time evt in use
    }
    else {                                          // the time event is armed
        isArmed = (uint8_t)1;
    }

    QS_BEGIN_NOLOCK_(QS_QF_TIMEEVT_REARM, QS::teObj_, this)
        QS_TIME_();                                               // timestamp
        QS_OBJ_(this);                               // this time event object
        QS_OBJ_(m_act);                                   // the active object
        QS_TEC_(m_ctr);                                 // the number of ticks
        QS_TEC_(m_interval);                                   // the interval
        QS_U8_(isArmed);                               // was the timer armed?
    QS_END_NOLOCK_()

    QF_INT_UNLOCK_();
    return isArmed;
}

//////////////////////////////////////////////////////////////////////////////
// Kernel selection based on QK_PREEMPTIVE
//
#ifdef QK_PREEMPTIVE

// "qk_pkg.h" ================================================================
                                    // QK internal interrupt locking/unlocking
#ifndef QF_INT_KEY_TYPE
    #define QK_INT_LOCK_KEY_
    #define QK_INT_LOCK_()      QF_INT_LOCK(dummy)
    #define QK_INT_UNLOCK_()    QF_INT_UNLOCK(dummy)
#else

    /// \brief This is an internal macro for defining the interrupt lock key.
    ///
    /// The purpose of this macro is to enable writing the same code for the
    /// case when interrupt key is defined and when it is not. If the macro
    /// #QF_INT_KEY_TYPE is defined, this internal macro provides the
    /// definition of the lock key variable. Otherwise this macro is empty.
    /// \sa #QF_INT_KEY_TYPE, #QF_INT_LOCK_, #QF_INT_UNLOCK_
    #define QK_INT_LOCK_KEY_    QF_INT_KEY_TYPE intLockKey_;

    /// \brief This is an internal macro for locking interrupts.
    ///
    /// The purpose of this macro is to enable writing the same code for the
    /// case when interrupt key is defined and when it is not. If the macro
    /// #QF_INT_KEY_TYPE is defined, this internal macro invokes #QF_INT_LOCK
    /// passing the key variable as the parameter. Otherwise #QF_INT_LOCK
    /// is invoked with a dummy parameter.
    /// \sa #QK_INT_LOCK_KEY_, #QK_INT_UNLOCK_
    #define QK_INT_LOCK_()      QF_INT_LOCK(intLockKey_)

    /// \brief This is an internal macro for unlocking interrupts.
    ///
    /// The purpose of this macro is to enable writing the same code for the
    /// case when interrupt key is defined and when it is not. If the macro
    /// #QF_INT_KEY_TYPE is defined, this internal macro invokes
    /// #QF_INT_UNLOCK passing the key variable as the parameter. Otherwise
    /// #QF_INT_UNLOCK is invoked with a dummy parameter.
    /// \sa #QK_INT_LOCK_KEY_, #QK_INT_LOCK_
    #define QK_INT_UNLOCK_()    QF_INT_UNLOCK(intLockKey_)
#endif

                                                   // package-scope objects...
#ifndef QK_NO_MUTEX
    extern uint8_t volatile QK_ceilingPrio_;    ///< QK mutex priority ceiling
#endif

// "qk.cpp" ==================================================================

// Public-scope objects ------------------------------------------------------
#if (QF_MAX_ACTIVE <= 8U)
#ifdef Q_USE_NAMESPACE
    QP::QPSet8  volatile QK_readySet_;                      // ready set of QK
#else
    QPSet8  volatile QK_readySet_;                          // ready set of QK
#endif
#else
#ifdef Q_USE_NAMESPACE
    QP::QPSet64 volatile QK_readySet_;                      // ready set of QK
#else
    QPSet64 volatile QK_readySet_;                          // ready set of QK
#endif
#endif

#ifdef Q_USE_NAMESPACE
}                                                              // namespace QP
#endif
                                         // start with the QK scheduler locked
extern "C" {

uint8_t volatile QK_currPrio_ = (uint8_t)(QF_MAX_ACTIVE + 1);
uint8_t volatile QK_intNest_;                 // start with nesting level of 0

}                                                                // extern "C"

#ifdef Q_USE_NAMESPACE
namespace QP {
#endif

//............................................................................
//lint -e970 -e971               ignore MISRA rules 13 and 14 in this function
char const Q_ROM * Q_ROM_VAR QK::getVersion(void) {
    static char const Q_ROM Q_ROM_VAR version[] = {
        (char)(((QP_VERSION >> 12U) & 0xFU) + (uint8_t)'0'),
        '.',
        (char)(((QP_VERSION >>  8U) & 0xFU) + (uint8_t)'0'),
        '.',
        (char)(((QP_VERSION >>  4U) & 0xFU) + (uint8_t)'0'),
        (char)((QP_VERSION          & 0xFU) + (uint8_t)'0'),
        '\0'
    };
    return version;
}
//............................................................................
void QF::init(void) {
    QK_init();           // QK initialization ("C" linkage, might be assembly)
}
//............................................................................
void QF::stop(void) {
    QF::onCleanup();                                       // cleanup callback
    // nothing else to do for the QK preemptive kernel
}
//............................................................................
void QF::run(void) {
    QK_INT_LOCK_KEY_

    QK_INT_LOCK_();
    QK_currPrio_ = (uint8_t)0;        // set the priority for the QK idle loop
    QK_SCHEDULE_();                      // process all events produced so far
    QK_INT_UNLOCK_();

    QF::onStartup();                                       // startup callback

    for (;;) {                                             // the QK idle loop
        QK::onIdle();                        // invoke the QK on-idle callback
    }
}
//............................................................................
void QActive::start(uint8_t prio,
                    QEvent const *qSto[], uint32_t qLen,
                    void *tls, uint32_t flags,
                    QEvent const *ie)
{
    Q_REQUIRE(((uint8_t)0 < prio) && (prio <= (uint8_t)QF_MAX_ACTIVE));

    m_eQueue.init(qSto, (QEQueueCtr)qLen);       // initialize the event queue
    m_prio = prio;
    QF::add_(this);                     // make QF aware of this active object

#if defined(QK_TLS) || defined(QK_EXT_SAVE)
    m_osObject = (uint8_t)flags;       // m_osObject contains the thread flags
    m_thread   = tls;      // contains the pointer to the thread-local-storage
#else
    Q_ASSERT((tls == (void *)0) && (flags == (uint32_t)0));
#endif

    init(ie);                                    // execute initial transition

    QS_FLUSH();                          // flush the trace buffer to the host
}
//............................................................................
void QActive::stop(void) {
    QF::remove_(this);                // remove this active object from the QF
}

// "qk_sched" ================================================================

#ifdef Q_USE_NAMESPACE
}                                                              // namespace QP
#endif

//............................................................................
// NOTE: the QK scheduler is entered and exited with interrupts LOCKED.
// QK_schedule_() is extern "C", so it does not belong to the QP namespace.
//
extern "C" {

#ifndef QF_INT_KEY_TYPE
void QK_schedule_(void) {
#else
void QK_schedule_(QF_INT_KEY_TYPE intLockKey_) {
#endif

#ifdef Q_USE_NAMESPACE
    using namespace QP;
#endif
                         // the QK scheduler must be called at task level only
    Q_REQUIRE(QK_intNest_ == (uint8_t)0);

           // determine the priority of the highest-priority task ready to run
    uint8_t p = QK_readySet_.findMax();

#ifdef QK_NO_MUTEX
    if (p > QK_currPrio_) {                        // do we have a preemption?
#else                                   // QK priority-ceiling mutexes allowed
    if ((p > QK_currPrio_) && (p > QK_ceilingPrio_)) {
#endif
        uint8_t pin = QK_currPrio_;               // save the initial priority
        QActive *a;
#ifdef QK_TLS                                    // thread-local storage used?
        uint8_t pprev = pin;
#endif
        do {
            QEvent const *e;
            a = QF::active_[p];                // obtain the pointer to the AO
            QK_currPrio_ = p;        // this becomes the current task priority

#ifdef QK_TLS                                    // thread-local storage used?
            if (p != pprev) {                      // are we changing threads?
                QK_TLS(a);                  // switch new thread-local storage
                pprev = p;
            }
#endif
            QS_BEGIN_NOLOCK_(QS_QK_SCHEDULE, QS::aoObj_, a)
                QS_TIME_();                                       // timestamp
                QS_U8_(p);                // the priority of the active object
                QS_U8_(pin);                         // the preempted priority
            QS_END_NOLOCK_()

            QK_INT_UNLOCK_();                         // unlock the interrupts

            e = a->get_();        // get the next event for this active object
            a->dispatch(e);                 // dispatch e to the active object
            QF::gc(e);              // garbage collect the event, if necessary

            QK_INT_LOCK_();
                             // determine the highest-priority AO ready to run
            if (QK_readySet_.notEmpty()) {
                p = QK_readySet_.findMax();
            }
            else {
                p = (uint8_t)0;
            }
#ifdef QK_NO_MUTEX
        } while (p > pin);         // is the new priority higher than initial?
#else                                   // QK priority-ceiling mutexes allowed
        } while ((p > pin) && (p > QK_ceilingPrio_));
#endif
        QK_currPrio_ = pin;                    // restore the initial priority

#ifdef QK_TLS                                    // thread-local storage used?
        if (pin != (uint8_t)0) {      // no extended context for the idle loop
            a = QF::active_[pin];           // the pointer to the preempted AO
            QK_TLS(a);                             // restore the original TLS
        }
#endif
    }
}

}                                                                // extern "C"

#ifdef Q_USE_NAMESPACE
namespace QP {
#endif

// "qk_mutex.cpp" ============================================================
#ifndef QK_NO_MUTEX

// package-scope objects -----------------------------------------------------
uint8_t volatile QK_ceilingPrio_;               // ceiling priority of a mutex

//............................................................................
QMutex QK::mutexLock(uint8_t prioCeiling) {
    QK_INT_LOCK_KEY_
    QK_INT_LOCK_();
    uint8_t mutex = QK_ceilingPrio_; // original QK priority ceiling to return
    if (QK_ceilingPrio_ < prioCeiling) {
        QK_ceilingPrio_ = prioCeiling;        // raise the QK priority ceiling
    }

    QS_BEGIN_NOLOCK_(QS_QK_MUTEX_LOCK, (void *)0, (void *)0)
        QS_TIME_();                                               // timestamp
        QS_U8_(mutex);                                // the original priority
        QS_U8_(QK_ceilingPrio_);               // the current priority ceiling
    QS_END_NOLOCK_()

    QK_INT_UNLOCK_();
    return mutex;
}
//............................................................................
void QK::mutexUnlock(QMutex mutex) {
    QK_INT_LOCK_KEY_
    QK_INT_LOCK_();

    QS_BEGIN_NOLOCK_(QS_QK_MUTEX_UNLOCK, (void *)0, (void *)0)
        QS_TIME_();                                               // timestamp
        QS_U8_(mutex);                                // the original priority
        QS_U8_(QK_ceilingPrio_);               // the current priority ceiling
    QS_END_NOLOCK_()

    if (QK_ceilingPrio_ > mutex) {
        QK_ceilingPrio_ = mutex;         // restore the saved priority ceiling
        QK_SCHEDULE_();
    }
    QK_INT_UNLOCK_();
}
#endif                                                          // QK_NO_MUTEX

#else                                                         // QK_PREEMPTIVE

// "qvanilla.cpp" ============================================================
// Package-scope objects -----------------------------------------------------
#if (QF_MAX_ACTIVE <= 8)
    QPSet8  volatile QF_readySet_;           // QF-ready set of active objects
#else
    QPSet64 volatile QF_readySet_;           // QF-ready set of active objects
#endif

//............................................................................
char const Q_ROM * Q_ROM_VAR QF::getPortVersion(void) {
    static const char Q_ROM version[] = "4.0.00";
    return version;
}
//............................................................................
void QF::init(void) {
    // nothing to do for the "vanilla" kernel
}
//............................................................................
void QActive::start(uint8_t prio,
                    QEvent const *qSto[], uint32_t qLen,
                    void *stkSto, uint32_t /*lint -e1904 stkSize */,
                    QEvent const *ie)
{
    Q_REQUIRE(((uint8_t)0 < prio) && (prio <= (uint8_t)QF_MAX_ACTIVE)
              && (stkSto == (void *)0));      // does not need per-actor stack

    m_eQueue.init(qSto, (QEQueueCtr)qLen);               // initialize QEQueue
    m_prio = prio;                // set the QF priority of this active object
    QF::add_(this);                     // make QF aware of this active object
    init(ie);                                    // execute initial transition

    QS_FLUSH();                          // flush the trace buffer to the host
}
//............................................................................
void QActive::stop(void) {
    QF::remove_(this);
}

//............................................................................
void QF::stop(void) {
    QF::onCleanup();                                       // cleanup callback
    // nothing else to do for the "vanilla" kernel
}
//............................................................................
void QF::run(void) {
    QF::onStartup();                                       // startup callback

    for (;;) {                                           // the bacground loop
        QF_INT_LOCK_KEY_
        QF_INT_LOCK_();
        if (QF_readySet_.notEmpty()) {
            uint8_t p = QF_readySet_.findMax();
            QActive *a = active_[p];
            QF_INT_UNLOCK_();

            QEvent const *e = a->get_();     // get the next event for this AO
            a->dispatch(e);                         // dispatch evt to the HSM
            gc(e);       // determine if event is garbage and collect it if so
        }
        else {
#ifndef QF_INT_KEY_TYPE
            onIdle();                                            // see NOTE01
#else
            onIdle(intLockKey_);                                 // see NOTE01
#endif                                                      // QF_INT_KEY_TYPE
        }
    }
}

//////////////////////////////////////////////////////////////////////////////
// NOTE01:
// QF::onIdle() must be called with interrupts LOCKED because the
// determination of the idle condition (no events in the queues) can change
// at any time by an interrupt posting events to a queue. The QF::onIdle()
// MUST enable interrups internally, perhaps at the same time as putting the
// CPU into a power-saving mode.
//

#endif                                                        // QK_PREEMPTIVE

//////////////////////////////////////////////////////////////////////////////
#ifdef Q_SPY

// "qs_pkg.h" ================================================================
/// \brief QS ring buffer counter and offset type
typedef uint16_t QSCtr;

/// \brief Internal QS macro to insert an un-escaped byte into
/// the QS buffer
////
#define QS_INSERT_BYTE(b_) \
    QS_ring_[QS_head_] = (b_); \
    ++QS_head_; \
    if (QS_head_ == QS_end_) { \
        QS_head_ = (QSCtr)0; \
    } \
    ++QS_used_;

/// \brief Internal QS macro to insert an escaped byte into the QS buffer
#define QS_INSERT_ESC_BYTE(b_) \
    QS_chksum_ = (uint8_t)(QS_chksum_ + (b_)); \
    if (((b_) == QS_FRAME) || ((b_) == QS_ESC)) { \
        QS_INSERT_BYTE(QS_ESC) \
        QS_INSERT_BYTE((uint8_t)((b_) ^ QS_ESC_XOR)) \
    } \
    else { \
        QS_INSERT_BYTE(b_) \
    }

/// \brief Internal QS macro to insert a escaped checksum byte into
/// the QS buffer
#define QS_INSERT_CHKSUM_BYTE() \
    QS_chksum_ = (uint8_t)~QS_chksum_; \
    if ((QS_chksum_ == QS_FRAME) || (QS_chksum_ == QS_ESC)) { \
        QS_INSERT_BYTE(QS_ESC) \
        QS_INSERT_BYTE((uint8_t)(QS_chksum_ ^ QS_ESC_XOR)) \
    } \
    else { \
        QS_INSERT_BYTE(QS_chksum_) \
    }


/// \brief Frame character of the QS output protocol
#define QS_FRAME    ((uint8_t)0x7E)

/// \brief Escape character of the QS output protocol
#define QS_ESC      ((uint8_t)0x7D)

/// \brief Escape modifier of the QS output protocol
///
/// The escaped byte is XOR-ed with the escape modifier before it is inserted
/// into the QS buffer.
#define QS_ESC_XOR  0x20

#ifndef Q_ROM_BYTE
    /// \brief Macro to access a byte allocated in ROM
    ///
    /// Some compilers for Harvard-architecture MCUs, such as gcc for AVR, do
    /// not generate correct code for accessing data allocated in the program
    /// space (ROM). The workaround for such compilers is to explictly add
    /// assembly code to access each data element allocated in the program
    /// space. The macro Q_ROM_BYTE() retrieves a byte from the given ROM
    /// address.
    ///
    /// The Q_ROM_BYTE() macro should be defined for the compilers that
    /// cannot handle correctly data allocated in ROM (such as the gcc).
    /// If the macro is left undefined, the default definition simply returns
    /// the argument and lets the compiler generate the correct code.
    #define Q_ROM_BYTE(rom_var_)   (rom_var_)
#endif

//............................................................................
extern uint8_t *QS_ring_;         ///< pointer to the start of the ring buffer
extern QSCtr QS_end_;                ///< offset of the end of the ring buffer
extern QSCtr QS_head_;         ///< offset to where next byte will be inserted
extern QSCtr QS_tail_;       ///< offset of where next event will be extracted
extern QSCtr QS_used_;       ///< number of bytes currently in the ring buffer
extern uint8_t QS_seq_;                        ///< the record sequence number
extern uint8_t QS_chksum_;             ///< the checksum of the current record
extern uint8_t QS_full_;              ///< the ring buffer is temporarily full

// "qs.cpp" ==================================================================
//............................................................................
uint8_t QS::glbFilter_[32];                                // global QS filter

//............................................................................
uint8_t *QS_ring_;                  // pointer to the start of the ring buffer
QSCtr QS_end_;                         // offset of the end of the ring buffer
QSCtr QS_head_;                  // offset to where next byte will be inserted
QSCtr QS_tail_;                 // offset of where next byte will be extracted
QSCtr QS_used_;                // number of bytes currently in the ring buffer
uint8_t QS_seq_;                                 // the record sequence number
uint8_t QS_chksum_;                      // the checksum of the current record
uint8_t QS_full_;                       // the ring buffer is temporarily full

//............................................................................
//lint -e970 -e971               ignore MISRA rules 13 and 14 in this function
char const Q_ROM * Q_ROM_VAR QS::getVersion(void) {
    static char const Q_ROM Q_ROM_VAR version[] = {
        (char)(((QP_VERSION >> 12U) & 0xFU) + (uint8_t)'0'),
        '.',
        (char)(((QP_VERSION >>  8U) & 0xFU) + (uint8_t)'0'),
        '.',
        (char)(((QP_VERSION >>  4U) & 0xFU) + (uint8_t)'0'),
        (char)((QP_VERSION          & 0xFU) + (uint8_t)'0'),
        '\0'
    };
    return version;
}
//............................................................................
void QS::initBuf(uint8_t sto[], uint32_t stoSize) {
    QS_ring_ = &sto[0];
    QS_end_  = (QSCtr)stoSize;
}
//............................................................................
void QS::filterOn(uint8_t rec) {
    if (rec == QS_ALL_RECORDS) {
        uint8_t i;
        for (i = (uint8_t)0; i < (uint8_t)sizeof(glbFilter_); ++i) {
            glbFilter_[i] = (uint8_t)0xFF;
        }
    }
    else {
        glbFilter_[rec >> 3] |= (uint8_t)(1U << (rec & 0x07));
    }
}
//............................................................................
void QS::filterOff(uint8_t rec) {
    if (rec == QS_ALL_RECORDS) {
        uint8_t i;
        for (i = (uint8_t)0; i < (uint8_t)sizeof(glbFilter_); ++i) {
            glbFilter_[i] = (uint8_t)0;
        }
    }
    else {
        glbFilter_[rec >> 3] &= (uint8_t)(~(1U << (rec & 0x07)));
    }
}
//............................................................................
void QS::begin(uint8_t rec) {
    QS_chksum_ = (uint8_t)0;                             // clear the checksum
    ++QS_seq_;                         // always increment the sequence number
    QS_INSERT_ESC_BYTE(QS_seq_)                   // store the sequence number
    QS_INSERT_ESC_BYTE(rec)                             // store the record ID
}
//............................................................................
void QS::end(void) {
    QS_INSERT_CHKSUM_BYTE()
    QS_INSERT_BYTE(QS_FRAME)
    if (QS_used_ > QS_end_) {                    // overrun over the old data?
        QS_tail_ = QS_head_;                 // shift the tail to the old data
        QS_used_ = QS_end_;                        // the whole buffer is used
    }
}
//............................................................................
void QS::u8(uint8_t format, uint8_t d) {
    QS_INSERT_ESC_BYTE(format)
    QS_INSERT_ESC_BYTE(d)
}
//............................................................................
void QS::u16(uint8_t format, uint16_t d) {
    QS_INSERT_ESC_BYTE(format)
    QS_INSERT_ESC_BYTE((uint8_t)d)
    d >>= 8;
    QS_INSERT_ESC_BYTE((uint8_t)d)
}
//............................................................................
void QS::u32(uint8_t format, uint32_t d) {
    QS_INSERT_ESC_BYTE(format)
    QS_INSERT_ESC_BYTE((uint8_t)d)
    d >>= 8;
    QS_INSERT_ESC_BYTE((uint8_t)d)
    d >>= 8;
    QS_INSERT_ESC_BYTE((uint8_t)d)
    d >>= 8;
    QS_INSERT_ESC_BYTE((uint8_t)d)
}

// "qs_.cpp" =================================================================
//............................................................................
void const *QS::smObj_;                  // local state machine for QEP filter
void const *QS::aoObj_;                   // local active object for QF filter
void const *QS::mpObj_;                     //  local event pool for QF filter
void const *QS::eqObj_;                      //  local raw queue for QF filter
void const *QS::teObj_;                     //  local time event for QF filter
void const *QS::apObj_;                    //  local object Application filter

QSTimeCtr volatile QS::tickCtr_;     // tick counter for the QS_QF_TICK record

//............................................................................
void QS::u8_(uint8_t d) {
    QS_INSERT_ESC_BYTE(d)
}
//............................................................................
void QS::u16_(uint16_t d) {
    QS_INSERT_ESC_BYTE((uint8_t)d)
    d >>= 8;
    QS_INSERT_ESC_BYTE((uint8_t)d)
}
//............................................................................
void QS::u32_(uint32_t d) {
    QS_INSERT_ESC_BYTE((uint8_t)d)
    d >>= 8;
    QS_INSERT_ESC_BYTE((uint8_t)d)
    d >>= 8;
    QS_INSERT_ESC_BYTE((uint8_t)d)
    d >>= 8;
    QS_INSERT_ESC_BYTE((uint8_t)d)
}
//............................................................................
//lint -e970 -e971               ignore MISRA rules 13 and 14 in this function
void QS::str_(char const *s) {
    while (*s != '\0') {
                                       // ASCII characters don't need escaping
        QS_chksum_ = (uint8_t)(QS_chksum_ + (uint8_t)(*s));
        QS_INSERT_BYTE((uint8_t)(*s))
        ++s;
    }
    QS_INSERT_BYTE((uint8_t)0)
}
//............................................................................
//lint -e970 -e971               ignore MISRA rules 13 and 14 in this function
void QS::str_ROM_(char const Q_ROM * Q_ROM_VAR s) {
    uint8_t b;
    while ((b = (uint8_t)Q_ROM_BYTE(*s)) != (uint8_t)0) {
                                       // ASCII characters don't need escaping
        QS_chksum_ = (uint8_t)(QS_chksum_ + b);
        QS_INSERT_BYTE(b)
        ++s;
    }
    QS_INSERT_BYTE((uint8_t)0)
}

// "qs_blk.cpp" ==============================================================
//............................................................................
// get up to *pn bytes of contiguous memory
uint8_t const *QS::getBlock(uint16_t *pNbytes) {
    uint8_t *block;
    if (QS_used_ == (QSCtr)0) {
        *pNbytes = (uint16_t)0;
        block = (uint8_t *)0;                  // no bytes to return right now
    }
    else {
        QSCtr n = (QSCtr)(QS_end_ - QS_tail_);
        if (n > QS_used_) {
            n = QS_used_;
        }
        if (n > (QSCtr)(*pNbytes)) {
            n = (QSCtr)(*pNbytes);
        }
        *pNbytes = (uint16_t)n;
        QS_used_ = (QSCtr)(QS_used_ - n);
        QSCtr t  = QS_tail_;
        QS_tail_ = (QSCtr)(QS_tail_ + n);
        if (QS_tail_ == QS_end_) {
            QS_tail_ = (QSCtr)0;
        }
        block = &QS_ring_[t];
    }
    return block;
}

// "qs_byte.cpp" =============================================================
//............................................................................
uint16_t QS::getByte(void) {
    uint16_t ret;
    if (QS_used_ == (QSCtr)0) {
        ret = QS_EOD;                                       // set End-Of-Data
    }
    else {
        ret = QS_ring_[QS_tail_];                    // set the byte to return
        ++QS_tail_;                                        // advance the tail
        if (QS_tail_ == QS_end_) {                        // tail wrap around?
            QS_tail_ = (QSCtr)0;
        }
        --QS_used_;                                      // one less byte used
    }
    return ret;                                      // return the byte or EOD
}

// "qs_f32.cpp" ==============================================================
//............................................................................
void QS::f32(uint8_t format, float f) {
    union F32Rep {
        float f;
        uint32_t u;
    } fu32;
    fu32.f = f;

    QS_INSERT_ESC_BYTE(format)
    QS_INSERT_ESC_BYTE((uint8_t)fu32.u)
    fu32.u >>= 8;
    QS_INSERT_ESC_BYTE((uint8_t)fu32.u)
    fu32.u >>= 8;
    QS_INSERT_ESC_BYTE((uint8_t)fu32.u)
    fu32.u >>= 8;
    QS_INSERT_ESC_BYTE((uint8_t)fu32.u)
}

// "qs_f64.cpp" ==============================================================
//............................................................................
void QS::f64(uint8_t format, double d) {
    union F64Rep {
        double d;
        struct UInt2 {
            uint32_t u1, u2;
        } i;
    } fu64;
    fu64.d = d;

    QS_INSERT_ESC_BYTE(format)

    QS_INSERT_ESC_BYTE((uint8_t)fu64.i.u1)
    fu64.i.u1 >>= 8;
    QS_INSERT_ESC_BYTE((uint8_t)fu64.i.u1)
    fu64.i.u1 >>= 8;
    QS_INSERT_ESC_BYTE((uint8_t)fu64.i.u1)
    fu64.i.u1 >>= 8;
    QS_INSERT_ESC_BYTE((uint8_t)fu64.i.u1)

    QS_INSERT_ESC_BYTE((uint8_t)fu64.i.u2)
    fu64.i.u2 >>= 8;
    QS_INSERT_ESC_BYTE((uint8_t)fu64.i.u2)
    fu64.i.u2 >>= 8;
    QS_INSERT_ESC_BYTE((uint8_t)fu64.i.u2)
    fu64.i.u2 >>= 8;
    QS_INSERT_ESC_BYTE((uint8_t)fu64.i.u2)
}

// "qs_mem.cpp" ==============================================================
//............................................................................
void QS::mem(uint8_t const *blk, uint8_t size) {
    QS_INSERT_BYTE((uint8_t)QS_MEM_T)
    QS_chksum_ = (uint8_t)(QS_chksum_ + (uint8_t)QS_MEM_T);
    QS_INSERT_ESC_BYTE(size)
    while (size != (uint8_t)0) {
        QS_INSERT_ESC_BYTE(*blk)
        ++blk;
        --size;
    }
}

// "qs_str.cpp" ==============================================================
//............................................................................
//lint -e970 -e971               ignore MISRA rules 13 and 14 in this function
void QS::str(char const *s) {
    QS_INSERT_BYTE((uint8_t)QS_STR_T)
    QS_chksum_ = (uint8_t)(QS_chksum_ + (uint8_t)QS_STR_T);
    while ((*s) != '\0') {
                                       // ASCII characters don't need escaping
        QS_INSERT_BYTE((uint8_t)(*s))
        QS_chksum_ = (uint8_t)(QS_chksum_ + (uint8_t)(*s));
        ++s;
    }
    QS_INSERT_BYTE((uint8_t)0)
}
//............................................................................
//lint -e970 -e971               ignore MISRA rules 13 and 14 in this function
void QS::str_ROM(char const Q_ROM * Q_ROM_VAR s) {
    QS_INSERT_BYTE((uint8_t)QS_STR_T)
    QS_chksum_ = (uint8_t)(QS_chksum_ + (uint8_t)QS_STR_T);
    uint8_t b;
    while ((b = (uint8_t)Q_ROM_BYTE(*s)) != (uint8_t)0) {
                                       // ASCII characters don't need escaping
        QS_INSERT_BYTE(b)
        QS_chksum_ = (uint8_t)(QS_chksum_ + b);
        ++s;
    }
    QS_INSERT_BYTE((uint8_t)0)
}

// "qs_u64.cpp" ==============================================================
#if (QS_OBJ_PTR_SIZE == 8) || (QS_FUN_PTR_SIZE == 8)

//............................................................................
void QS::u64_(uint64_t d) {
    QS_INSERT_ESC_BYTE((uint8_t)d)
    d >>= 8;
    QS_INSERT_ESC_BYTE((uint8_t)d)
    d >>= 8;
    QS_INSERT_ESC_BYTE((uint8_t)d)
    d >>= 8;
    QS_INSERT_ESC_BYTE((uint8_t)d)
    d >>= 8;
    QS_INSERT_ESC_BYTE((uint8_t)d)
    d >>= 8;
    QS_INSERT_ESC_BYTE((uint8_t)d)
    d >>= 8;
    QS_INSERT_ESC_BYTE((uint8_t)d)
    d >>= 8;
    QS_INSERT_ESC_BYTE((uint8_t)d)
}
//............................................................................
void QS::u64(uint8_t format, uint64_t d) {
    QS_INSERT_ESC_BYTE(format)
    u64_(d);
}

#endif

#endif                                                                // Q_SPY

#ifdef Q_USE_NAMESPACE
}                                                              // namespace QP
#endif

//////////////////////////////////////////////////////////////////////////////
// NOTE01:
// QF::onIdle() must be called with interrupts LOCKED because the
// determination of the idle condition (no events in the queues) can change
// at any time by an interrupt posting events to a queue. The QF::onIdle()
// MUST enable interrups internally, perhaps at the same time as putting the
// CPU into a power-saving mode.
//


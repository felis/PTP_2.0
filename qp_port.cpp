//////////////////////////////////////////////////////////////////////////////
// Product: QP/C++ port to Arduino, cooperative "vanilla" kernel, no Q-SPY
// Last Updated for QP ver: 4.1.06 (modified to fit in one file)
// Date of the Last Update: Jan 04, 2011
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

Q_DEFINE_THIS_MODULE(qp_port)

//............................................................................
//extern "C" void loop() {
//    QF::run();         // run the application, NOTE: QF::run() does not return
//}

//............................................................................
// This QP framework does NOT use new or delete, but the WinAVR/avr-g++
// compiler generates somehow a reference to the operator delete for every
// class with a virtual destructor. QP declares virtual destructors, so to
// satisfy the linker the following dummy definition of the operator
// delete is provided. This operator should never be actually called.
//
/*void operator delete(void *) {
    Q_ERROR();               // this operator should never be actually called
}*/

void Q_onAssert(char const Q_ROM * const Q_ROM_VAR file, int line) {
    cli();                                              // lock all interrupts
    //USER_LED_ON();                                  // User LED permanently ON
    asm volatile ("jmp 0x0000");    // perform a software reset of the Arduino
}

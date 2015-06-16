/****************************************************************************
Copyright (C) Cambridge Silicon Radio Limited 2013-2014
Part of ADK 3.5

FILE NAME
    sink_ir_remote_control.h

DESCRIPTION
    Header file for interface with Infrared Remote Controllers
*/

#ifndef _SINK_IR_REMOTE_CONTROL_H_
#define _SINK_IR_REMOTE_CONTROL_H_

/* firmware includes */
#include <infrared.h>

/* Application includes */
#include "sink_input_manager.h"

/*!
   A infra-red 'button has been pressed or releases. The message content
   is a MessageInfraRedEvent.
   This message is sent to the task registered with MessageInfraRedTask()
*/
#define MESSAGE_INFRARED_EVENT (SYSTEM_MESSAGE_BASE_ + 58)

/*
    Defines the Timer IDs to differentiate which timer has fired
*/
typedef enum
{
    IR_RC_TIMER_SHORT=0,   /* The SHORT timer has fired whilst the button is still pressed */
    IR_RC_TIMER_LONG,      /* The LONG timer has fired whilst the button is still pressed */
    IR_RC_TIMER_VLONG,     /* The VLONG timer has fired whilst the button is still pressed */
    IR_RC_TIMER_VVLONG     /* The VVLONG timer has fired whilst the button is still pressed */
} irRcTimerID_t;


/*
    Defines the IR RC button events
    These IDs are the actual message ID's as identified by the MessageLoop() scheduler
*/
typedef enum
{
    IR_RC_BUTTON_TIMER_MSG=100, /* Sent when a button duration (SHORT, LONG, VLONG, VVLONG) timer fires (Given ID 100 so it does not conflict with IR message IDs from firmware - this message is only used by the IR handler) */
    IR_RC_BUTTON_REPEAT_MSG     /* Sent when a button REPEAT timer fires */
} irRcMessageID_t;


/* 
    Defines the data structure for BUTTON EVENT messages
*/
typedef struct
{
    uint16              addr;   /* Address to identify the RC the button press was generated by */
    uint16              mask;   /* Identifies which button is currently pressed down */
    irRcTimerID_t       timer;  /* Identifies the button timer that has fired (Only used for IR_RC_BUTTON_TIMER_MSG) */
} IR_RC_BUTTON_EVENT_MSG_T;


/* 
    Defines the Infrared lookup table for converting IR codes sent from a 
    known remote controller (that matches the remote address) to an input mask
    based on the IR code that was sent.
*/
typedef struct
{
    unsigned ir_code:8;         /* The infrared code sent by the remote controller that converts to *input_id* for the input manager to process */
    unsigned reserved:4;		/* Reserved for future use */
    unsigned input_id:4;        /* The input ID the IR code translates to (allows up to 16 RC buttons to be supported) */
    uint16 remote_address;      /* The address of the remote control that sent the IR code (an Infrared remote may use different addresses for different buttons, so need to ensure the code was actually sent from the known RC) */
} irLookupTableConfig_t;


/* 
    Defines the data structure of the Infrared RC configuration data
*/
typedef struct
{
    infrared_protocol       protocol:16;              /* Defines which IR protocol the application supports (can only support one protocol; either NEC or RC5) */
    uint16                  max_learning_codes;       /* The maximum number of IR codes the application can learn */
    uint16                  learning_mode_timeout;    /* Failsafe for turning off the IR learning mode (ms) */
    uint16                  learning_mode_reminder;   /* A reminder tone can be generated to indicate learning mode is active, this will be repeated every timeout (ms) */
    uint16                  ir_pio;                   /* The PIO interfacing the InfraRed hardware */
    irLookupTableConfig_t   lookup_table[1];          /* Stores the lookup table configured for the IR remote */
} irRcConfig_t;


/*
    Task data required by the Infrared input monitor
*/
typedef struct
{
    TaskData            task;                   /* The task that is responsible for communicating with the BLE HID RC */
    Task                input_manager;          /* The "input manager" task that the "BLE Input Monitor" will notify of all valid input events sent from the BLE HID RC */
    
    /* IR config */
    timerConfig_t       *timers;                /* The timers configured (for SHORT/LONG/VLONG etc.), these are part of the InputManager config so just obtain a pointer to that memory area rather than creating a duplicate local copy (makes references to those timers easier to read) */
    uint16              size_lookup_table;      /* The number of entries in the configured lookup table (DOES NOT include learning mode lookup table) */
    irRcConfig_t        *config;                /* The config data for the IR remote control feature */
    
    /* IR learning mode */
    uint16                 num_learnt_codes;    /* The number of IR codes that have been learnt */
    irLookupTableConfig_t  *learnt_codes;       /* Points to the memory area used to store the learnt codes (if any exist) */
    uint16                 learn_this_input_id; /* When in learning mode, this is the input ID to learn */
    
    /* Runtime data */
    uint16              button_mask;            /* Used to keep track of which button is held down (if any) */    
    unsigned            button_state:4;         /* Used to keep track of the button state */
    
    /* Control flags */
    unsigned            learning_mode:1;        /* Flag to indicate whether or not "learning mode" is in progess */
    unsigned            reserved:11;
    
} irRCTaskData_t; 


/****************************************************************************
NAME    
    irCanLearnNewCode
    
DESCRIPTION
    Function to indicate whether or not the IR controller can learn a new
    code. If the maximum number of codes have already been learnt, will
    not allow a new code to be learnt.
*/
bool irCanLearnNewCode(void);


/****************************************************************************
NAME
    irStartLearningMode

DESCRIPTION
    Function to start the IR Learning mode (if state allows).
*/
bool irStartLearningMode(void);


/****************************************************************************
NAME
    handleIrLearningModeReminder

DESCRIPTION
    Function to handle the learning mode reminder event
*/
void handleIrLearningModeReminder(void);


/****************************************************************************
NAME
    irStopLearningMode

DESCRIPTION
    Function to stop the IR Learning mode
*/
void irStopLearningMode(void);


/****************************************************************************
NAME
    irClearLearntCodes

DESCRIPTION
    Function to clear the IR codes that have been learnt through the learning
    mode
*/
void irClearLearntCodes(void);


/****************************************************************************
NAME    
    initIrInputMonitor
    
DESCRIPTION
    Function to initialise the Infrared Remote Controller Task
*/
void initIrInputMonitor( timerConfig_t * timers );


/* Macro used by the IR "Input Monitor" to generate messages to send itself */
#define MAKE_IR_RC_MESSAGE(TYPE) TYPE##_T *message = PanicUnlessNew(TYPE##_T);


#endif /* _SINK_IR_REMOTE_CONTROL_H_ */
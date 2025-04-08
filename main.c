/******************************************************************************
* File Name:   main.c
*
* Description: This is the source code for the CE231732 PSoC 4 MCU: PWM
*              triggering a DMA Channel Example for ModusToolbox.
*
* Related Document: See README.md
*
*
*******************************************************************************
* Copyright 2020-2025, Cypress Semiconductor Corporation (an Infineon company) or
* an affiliate of Cypress Semiconductor Corporation.  All rights reserved.
*
* This software, including source code, documentation and related
* materials ("Software") is owned by Cypress Semiconductor Corporation
* or one of its affiliates ("Cypress") and is protected by and subject to
* worldwide patent protection (United States and foreign),
* United States copyright laws and international treaty provisions.
* Therefore, you may use this Software only as provided in the license
* agreement accompanying the software package from which you
* obtained this Software ("EULA").
* If no EULA applies, Cypress hereby grants you a personal, non-exclusive,
* non-transferable license to copy, modify, and compile the Software
* source code solely for use in connection with Cypress's
* integrated circuit products.  Any reproduction, modification, translation,
* compilation, or representation of this Software except as specified
* above is prohibited without the express written permission of Cypress.
*
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND,
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress
* reserves the right to make changes to the Software without notice. Cypress
* does not assume any liability arising out of the application or use of the
* Software or any product or circuit described in the Software. Cypress does
* not authorize its products for use in any products where a malfunction or
* failure of the Cypress product may reasonably be expected to result in
* significant property damage, injury or death ("High Risk Product"). By
* including Cypress's product in a High Risk Product, the manufacturer
* of such system or application assumes all risk of such use and in doing
* so agrees to indemnify Cypress against all liability.
*******************************************************************************/


/*******************************************************************************
 * Include header files
 ******************************************************************************/

#include "cy_pdl.h"
#include "cybsp.h"

/*******************************************************************************
* Macros
********************************************************************************/
#define USER_PWM_CNT             ((TCPWM_CNT_Type*) &USER_PWM_HW->CNT[USER_PWM_NUM])
#define CHANN_NUM                (0UL)
#define ARRAY_SIZE               (256u)
#define BREATHING_CHANGE         (512u)

/*******************************************************************************
* Function Prototypes
********************************************************************************/
void arrayInit(void);

/*******************************************************************************
* Global Variables
********************************************************************************/

/* This array holds all the compare values that will be loaded onto the PWM
*  to create the breathing pattern */
uint32_t compareVal[ARRAY_SIZE];

/*******************************************************************************
* Function Name: main
********************************************************************************
* Summary:
* This is the main function for CM0 Plus CPU. It does the following functions -
*    1. Generate the compare array
*    2. Initialize and enable the PWM block
*    3. Connect PWM overflow output trigger to DMA input trigger
*    4. Set DMA ping descriptor source address as the compare array
*    5. Set DMA ping descriptor destination as TCPWM compare register
*    6. Initialize and enable the DMA channel and DMA block
*
* Parameters:
*  void
*
* Return:
*  int
*
*******************************************************************************/
int main(void)
{
    cy_rslt_t result;

    /* Initialize the device and board peripherals */
    result = cybsp_init() ;
    if (result != CY_RSLT_SUCCESS)
    {
        CY_ASSERT(0);
    }

    /* Initialize the compareVal array */
    arrayInit();

    /* Enable global interrupts */
    __enable_irq();

    /* Initialize PWM using the config structure generated using device configurator*/
    if (CY_TCPWM_SUCCESS != Cy_TCPWM_PWM_Init(USER_PWM_HW, USER_PWM_NUM, &USER_PWM_config))
    {
        CY_ASSERT(0);
    }

    /* Enable the initialized PWM */
    Cy_TCPWM_PWM_Enable(USER_PWM_HW, USER_PWM_NUM);

    /* Start the PWM */
    Cy_TCPWM_TriggerReloadOrIndex(USER_PWM_HW, USER_PWM_MASK);

    /* Configure DMA Descriptor to change the PWM compare value per breathingLed array */
    USER_DMA_ping_config.srcAddress = &compareVal;
    USER_DMA_ping_config.dstAddress = (void *) &(USER_PWM_CNT->CC);
    /* Initialize the ping descriptor for channel 0 */
    if (CY_DMAC_SUCCESS != Cy_DMAC_Descriptor_Init(DMAC, CHANN_NUM, CY_DMAC_DESCRIPTOR_PING, &USER_DMA_ping_config))
    {
        CY_ASSERT(0);
    }
    /* Setup and enable DMAC channel 0 */
    if (CY_DMAC_SUCCESS != Cy_DMAC_Channel_Init(DMAC, CHANN_NUM, &USER_DMA_channel_config))
    {
        CY_ASSERT(0);
    }
    Cy_DMAC_Channel_Enable(DMAC, 0UL);

    /* Enable DMA hardware block */
    Cy_DMAC_Enable(USER_DMA_HW);

    for (;;)
    {
    }
}

/*******************************************************************************
* Function Name: arrayInit
********************************************************************************
* Summary:
* This function creates an array with increasing and decreasing compare values to
* generate the breathing effect.
*
* Parameters:
*  void
*
* Return:
*  void
*
*******************************************************************************/
void arrayInit(void)
{
    uint32_t i=0u;
    /* Creating the array with increasing and decreasing compare value to generate the breathing */
    compareVal[i] = 0;

    for(i=1u; i < ARRAY_SIZE>>1; i++)
    {
        /* Increase compare values */
        compareVal[i] = compareVal[i-1u] + BREATHING_CHANGE;
    }
    for(i=(ARRAY_SIZE>>1); i < ARRAY_SIZE; i++)
    {
        /* Decrease compare value */
        compareVal[i] = compareVal[i-1u] - BREATHING_CHANGE;
    }
}

/* [] END OF FILE */
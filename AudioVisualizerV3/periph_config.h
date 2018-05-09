
/*
 * periph_config.h
 *
 *  Created on: Apr 25, 2018
 *      Author: Andrew Teta
 */

#ifndef PERIPH_CONFIG_H_
#define PERIPH_CONFIG_H_

//#define BAUD9600
//#define BAUD19200
//#define BAUD38400
#define BAUD115200

void configureClocks(void);
void configureGPIO(void);
void configureADC(void);
void configureUART(void);

#endif /* PERIPH_CONFIG_H_ */

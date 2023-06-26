/*
 * 1-Version.h
 *
 *  Created on: 6 de mai de 2022
 *      Author: r
 */

#ifndef INC_1_VERSION_H_
#define INC_1_VERSION_H_

#define Firm_Mod "Datalogger 2039"
#define Firm_Ver "0.0.8 Beta"
#define byRef "Rtek1000"
#define byYear "2023"

#endif /* INC_1_VERSION_H_ */

/*
 * Version control:
 * 0.0.3 - Added Data conversion to base-64
 *       - Added Data export selector (base-16:'Dat'/base-64:'dat'/csv:'CSV')
 *       - Added LCD failure check
 *       - Changed LCD initialization using datasheet parameters
 *       - Added CSV file export
 *       - Changed Event datetime export
 *       - Changed Language config navigation
 *       - Changed DecimalSeparator bug
 *       - Changed DateFormat bug
 *       - Changed FileCntCsv bug
 *       - Changed FileCntDat bug
 *       - Changed temperature_unit bug
 *       - Changed Export bug (USB off)
 *       - Changed Export data header (added language_current & file_type)
 *       - Changed LCD SPI speed (From 10.5MHz to 21MHz)
 *
 * 0.0.2 - Changed PrintScreen bug (file header, cannot open on Windows)
 *       - Changed PrintScreen bug (USB off)
 *       - Added 1-Version.h file (This one)
 *
 * 0.0.1 - Prelimilary version
 *
 */

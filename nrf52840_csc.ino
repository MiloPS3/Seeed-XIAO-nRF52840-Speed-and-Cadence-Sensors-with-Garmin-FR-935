/*********************************************************************
 This is an example for our nRF52 based Bluefruit LE modules

 Pick one up today in the adafruit shop!

 Adafruit invests time and resources providing this open source code,
 please support Adafruit and open-source hardware by purchasing
 products from Adafruit!

 MIT license, check LICENSE for more information
 All text above, and the splash screen below must be included in
 any redistribution
*********************************************************************/
#include <bluefruit.h>

 byte csc_data[11] = { 0b00000011, 0,0,0,0,   0,0,   0,0,   0,0};
 
 //Cumulative Wheel Revolutions
 uint32_t cwr=0;
 //Last Wheel Event Time
 uint16_t lwet=0;
 //Cumulative Crank Revolutions
 uint16_t ccr=0;
 //Last Crank Event Time
 uint16_t lcet=0;

BLEService        csc_s = BLEService(UUID16_SVC_CYCLING_SPEED_AND_CADENCE);		//1816
BLECharacteristic csc_c = BLECharacteristic(UUID16_CHR_CSC_MEASUREMENT);		//2A5B
BLECharacteristic csc_f_c = BLECharacteristic(UUID16_CHR_CSC_FEATURE);			//2a5c

BLEDis bledis;    // DIS (Device Information Service) helper class instance
BLEBas blebas;    // BAS (Battery Service) helper class instance

void setup()
{
  Serial.begin(115200);
  while ( !Serial ) delay(10);   // for nrf52840 with native usb//
  delay(1000);

  Serial.println("Bluefruit52 CSC Example");
  Serial.println("-----------------------\n");

  // Initialise the Bluefruit module
  Serial.println("Initialise the nRF52 module");
  Bluefruit.begin();

  // Set the connect/disconnect callback handlers
  Bluefruit.Periph.setConnectCallback(connect_callback);
  Bluefruit.Periph.setDisconnectCallback(disconnect_callback);

  // Configure and Start the Device Information Service
  Serial.println("Configuring the Device Information Service");
  bledis.setManufacturer("Adafruit Industries");
  bledis.setModel("Bluefruit Feather52");
  bledis.begin();

  // Start the BLE Battery Service and set it to 100%
  Serial.println("Configuring the Battery Service");
  blebas.begin();
  blebas.write(100);

  // Setup the Heart Rate Monitor service using
  // BLEService and BLECharacteristic classes
  Serial.println("Configuring the Heart Rate Monitor Service");
  setupCSC();

  // Setup the advertising packet(s)
  Serial.println("Setting up the advertising payload(s)");
  startAdv();

  Serial.println("Ready Player One!!!");
  Serial.println("\nAdvertising");
}

void startAdv(void)
{
  // Advertising packet
  Bluefruit.Advertising.addFlags(BLE_GAP_ADV_FLAGS_LE_ONLY_LIMITED_DISC_MODE);
  Bluefruit.Advertising.addTxPower();

  // Include CSC Service UUID
  Bluefruit.Advertising.addService(csc_s);

  // Include Name
  Bluefruit.Advertising.addName();
  
  /* Start Advertising
   * - Enable auto advertising if disconnected
   * - Interval:  fast mode = 20 ms, slow mode = 152.5 ms
   * - Timeout for fast mode is 30 seconds
   * - Start(timeout) with timeout = 0 will advertise forever (until connected)
   * 
   * For recommended advertising interval
   * https://developer.apple.com/library/content/qa/qa1931/_index.html   
   */
  Bluefruit.Advertising.restartOnDisconnect(true);
  Bluefruit.Advertising.setInterval(32, 244);    // in unit of 0.625 ms
  Bluefruit.Advertising.setFastTimeout(30);      // number of seconds in fast mode
  Bluefruit.Advertising.start(0);                // 0 = Don't stop advertising after n seconds  
}

void setupCSC(void)
{
//	MANDATORY
 
 
  csc_s.begin();
  csc_c.setProperties(CHR_PROPS_NOTIFY);
  csc_c.setPermission(SECMODE_OPEN, SECMODE_NO_ACCESS);
  csc_c.setFixedLen(11);//11
  csc_c.setCccdWriteCallback(cccd_callback);  
  csc_c.begin();
  csc_c.write(csc_data, 11);
  csc_f_c.setProperties(CHR_PROPS_READ);
  csc_f_c.setPermission(SECMODE_OPEN, SECMODE_NO_ACCESS);
  csc_f_c.setFixedLen(2);
  csc_f_c.begin();
  csc_f_c.write16(0b0000000000000011);    
}

void connect_callback(uint16_t conn_handle)
{
  // Get the reference to current connection
  BLEConnection* connection = Bluefruit.Connection(conn_handle);

  char central_name[32] = { 0 };
  connection->getPeerName(central_name, sizeof(central_name));

  Serial.print("Connected to ");
  Serial.println(central_name);
}

/**
 * Callback invoked when a connection is dropped
 * @param conn_handle connection where this event happens
 * @param reason is a BLE_HCI_STATUS_CODE which can be found in ble_hci.h
 */
void disconnect_callback(uint16_t conn_handle, uint8_t reason)
{
  (void) conn_handle;
  (void) reason;

  Serial.print("Disconnected, reason = 0x"); Serial.println(reason, HEX);
  Serial.println("Advertising!");
}

void cccd_callback(uint16_t conn_hdl, BLECharacteristic* chr, uint16_t cccd_value)
{
    // Display the raw request packet
    Serial.print("CCCD Updated: ");
    Serial.print(cccd_value);
    Serial.println("");
    if (chr->uuid == csc_c.uuid) {
        if (chr->notifyEnabled(conn_hdl)) {
            Serial.println("CSC Measurement 'Notify' enabled");
        } else {
            Serial.println("CSC Measurement 'Notify' disabled");
        }
    }
}

uint16_t a=0,b=0 ;

void loop()
{
  digitalToggle(LED_RED);
  
  if ( Bluefruit.connected() ) {
    

	if ( csc_c.notify(csc_data, sizeof(csc_data)) ){
	//csc_data[0]=3;						// speed + cadence
	csc_data[1]=((int8_t)cwr);
	csc_data[2]=((int8_t)(cwr>> 8));
	csc_data[3]=((int8_t)(cwr>> 16));
	csc_data[4]=((int8_t)(cwr>> 24));
	csc_data[5]=((int8_t)lwet);
	csc_data[6]=((int8_t)(lwet>> 8));
	csc_data[7]=((int8_t)ccr);
	csc_data[8]=((int8_t)(ccr>> 8));
	csc_data[9]=((int8_t)lcet);
	csc_data[10]=((int8_t)(lcet>> 8));	
	
	csc_c.notify(csc_data, sizeof(csc_data));
	
	Serial.print("updated to: "); 
	Serial.print(csc_data[0]);
	Serial.print(",");
	Serial.print(cwr);
	Serial.print(","); 
	Serial.print(lwet); 
	Serial.print(",");
	Serial.print(ccr);
	Serial.print(","); 
	Serial.print(lcet);
	Serial.println(""); 

	cwr=cwr+1;
	lwet=lwet+1000-b;
	b=b+20;
	if(b>800)b=0;
	ccr=ccr+1;
	lcet=lcet+1000-a;
	a=a+7;
	if(a>400)a=0;	
	
	
	
	}
	
  }
 
	
  // Only send update once per second
  delay(1000);
}

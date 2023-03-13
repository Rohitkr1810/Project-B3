//Including Libraries for fingerprint library, lcd display, keypad, eeprom.
#include <Adafruit_Fingerprint.h>
#include <LiquidCrystal_I2C.h>

#include <Keypad.h>
#include <EEPROM.h>

#if (defined(__AVR__) || defined(ESP8266)) && !defined(__AVR_ATmega2560__)
SoftwareSerial mySerial(2, 3);
#else
#define mySerial Serial1
#endif

//Own variable delcarations
int id; //id given by the voter that is assigned to his finger image.
int entered=0; //counter to check whether id successfully entered.
int start=0;
const byte ROWS = 4; //four rows represents rows in keypad
const byte COLS = 4; //four columns represents coloumns in keypad
char keys[ROWS][COLS] = 
{
  {'1','2','3','A'},
  {'4','5','6','B'},
  {'7','8','9','C'},
  {'*','0','#','D'}
}; //Declaration of buttons in 2-D array of matrix keypad
byte rowPins[ROWS] = {11, 10, 9, 8}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {7, 6, 5, 4}; //connect to the column pinouts of the keypad
String input; //variable to store a string that is entered by the user.
int counter;
long fn;
char op;
char option;
const int maxLength=5; //It defines the max length of the id that can be entered by the user.
int intInput;
int idReturned;
int voted;//Counter to check whether the vote is casted successfully or not for each vote
//EEPROM.write(805,0);
//int address_id = EEPROM.read(805);
int address_id =0;
 //This represents addresses in the eeprom of the arduino to store the id's of voted people 

//Functions

bool searchInEEPROM(int uniId){ 
  for(int i=0; i<EEPROM.length(); i++){
    if(EEPROM.read(i) == uniId){
      return true;
    }
  }
  return false;
}

void storingId(int ID){
  EEPROM.write(address_id , ID);
  address_id++;
  EEPROM.put(805,address_id);
}

//EEPROM Variables
int partyA = EEPROM.read(800);
int partyB = EEPROM.read(801);
int partyC = EEPROM.read(802);
int partyD = EEPROM.read(803);
int totalVotes = EEPROM.read(804);

//COMPONENTS DELCARATION that is conneced or used in the kit.
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
LiquidCrystal_I2C lcd(0x27,20,4);
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

//LCD Display Functions
//Lcd funtions
void lcdWelcome(){
  lcd.setCursor(0, 0);
  lcd.print("Welcome");
  lcd.setCursor(0,1);
  lcd.print("Project");
  delay(3000);
  lcd.clear();
}

//the starting screen shown after switching the device on or a successful vote.
void startingScreen(){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("1. Enroll");
  lcd.setCursor(0,1);
  lcd.print("2. Identify");
  option = keypad.getKey();
}

//Setup code to verify that fingerprint sensor, lcd display all are working
void setup() {
  pinMode(A0,INPUT);                      
  lcd.init();
  lcd.backlight();
  EEPROM.begin();
  Serial.begin(9600);
  lcdWelcome(); //This is a welcome screen that is shown when the system
  //turned on for the first time.
  while (!Serial);  
  delay(100);
  Serial.println("\n\nAdafruit Fingerprint sensor enrollment");

  // set the data rate for the sensor serial port
  finger.begin(57600);

  if (finger.verifyPassword()) {
    Serial.println("Found fingerprint sensor!");
  } else {
    Serial.println("Did not find fingerprint sensor :(");
    while (1) { delay(1); }
  }

  finger.getParameters();
 
}

//Keypad Functions
bool IsNumber(char key)//this checks whether the given number is valid integer or not.
{
  if(key >= '0' && key <= '9')
   return true;

   return false;
}

void ProcessNumber(char key)//When we give integer by integer we need to process that
//integers to store that in to id.
{
  if(counter >= maxLength)
  return;
  
  counter++;
  input += key;
  if(op == ' ')  
   lcd.setCursor(2,0);
   else
    lcd.setCursor(0,1);

   lcd.print(input);
}

//Enter operation to assign input to id
int Enter(char key)
{
  //increamenting the entered counter to make sure the id is fully entered. 
  entered++;
  if(input.length() <= 0)
  return 0;

  long sn = input.toInt();
  return sn;
}

//to reset the input entered
void Reset()
{
  input ="\0";
}

//to get the input from matrix keypad
void getInput(){
  char key = keypad.getKey();
  if(key)
  {
    if(IsNumber(key))
    {
      ProcessNumber(key);
    }
    
    else if(key == 'D')
    {
     intInput = Enter(key);
     id = intInput;
    }
    else if(key == 'C')
    {
      Reset();
    }
  }
}

void loop() {
  // put your main code here, to run repeatedly:
  startingScreen();
  if(option == '1'){
    while(!getFingerprintEnroll());
  }
  if(option == '2'){
    idReturned=0;
    voted =0;
    //int returnedID;
    while(idReturned == 0){
      while(!getFingerprintID());
    }
    bool isVoterVoted = searchInEEPROM(finger.fingerID);
    if(isVoterVoted == true){
      voterAlreadyVoted();
    }
    else{
      continouingToVote(finger.fingerID);
    }
    
  }
  if(option =='3'){
    //Call couting function
    countingProcess();
  }
}

//Voting Functions
int waitForButtonPress(){
  lcd.clear();
  lcd.setCursor(0,0);
  delay(100);
  lcd.print("Cast your Vote");
  int vote;
  while(1){
    vote = analogRead(A0);
    if(vote>=50 && vote<=100){
      return 1;
    }
    if(vote>=330 && vote<=380){
      return 2;
    }
    else if(vote >=640 && vote<=660) {
      return 3;
    }
    else if(vote >670) {
      return 4;
    }
    delay(100);
  }
}

void processVote(int temp){
  if(temp ==1){
    partyA++;
    totalVotes++;
    EEPROM.write(800, partyA);
    EEPROM.write(804, totalVotes);
    delay(100);
  }
  else if(temp ==2){
    partyB++;
    totalVotes++;
    EEPROM.write(801, partyB);
    EEPROM.write(804, totalVotes);
    delay(100);
  }
  else if(temp ==3){
    partyC++;
    totalVotes++;
    EEPROM.write(802, partyC);
    EEPROM.write(804, totalVotes);
    delay(100);
  }
  else if(temp ==4){
    partyD++;
    totalVotes++;
    EEPROM.write(803, partyD);
    EEPROM.write(804, totalVotes);
    delay(100);
  }
  
}

void continouingToVote(int voterID){
  int returnedVote = waitForButtonPress();
  processVote(returnedVote);
  lcd.clear();
  lcd.print("Voted");
  delay(2000);
  lcd.clear();
  lcd.print("Processing...");
  delay(1000);
  storingId(voterID);
  successfullVote();
}

void successfullVote(){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Voted Successfully");
  delay(2000);
}

void voterAlreadyVoted(){
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("This Voter");
  lcd.setCursor(0,1);
  lcd.print("already Voted");
  delay(3000);
}

void countingProcess(){
int pA = EEPROM.read(800);
int pB = EEPROM.read(801);
int pC = EEPROM.read(802);
int pD = EEPROM.read(803);
int tV = EEPROM.read(804);
  
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("A-");
  lcd.setCursor(2,0);
  lcd.print(pA);
  lcd.setCursor(4,0);
  lcd.print("B-");
  lcd.setCursor(6,0);
  lcd.print(pB);
  lcd.setCursor(8,0);
  lcd.print("C-");
  lcd.setCursor(10,0);
  lcd.print(pC);
  lcd.setCursor(12,0);
  lcd.print("D-");
  lcd.setCursor(14,0);
  lcd.print(pD);
  lcd.setCursor(0,1);
  lcd.print("Total Votes-");
  lcd.setCursor(13,1);
  lcd.print(tV);
  delay(3000);
}

//Enrolling Function
uint8_t getFingerprintEnroll() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Enter ID: ");
  lcd.setCursor(0,1);
  while(entered == 0){
   char key = keypad.getKey();
  if(key)
  {
    if(IsNumber(key))
    {
      //if the entered key is a number process the key and number.
      ProcessNumber(key);
    }
    
    else if(key == 'D')
    {
     intInput = Enter(key);
     id = intInput;
     intInput=0;
     input ="\0";
     counter =0;
    }
    else if(key == 'C')
    {
      Reset();
    }
  }
  }
  // to make sure for smooth voting process entered is again made zero;
  entered=0;
  int p = -1;
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Enroll ID: "); lcd.print(id);
  lcd.setCursor(0,1);
  lcd.print("Place Finger");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Remove Finger");
  delay(1000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  p = -1;
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Place same");
  lcd.setCursor(0,1);
  lcd.print("finger again");
  delay(1000);
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.print(".");
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      break;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      break;
    default:
      Serial.println("Unknown error");
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK converted!

  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    Serial.println("Prints matched!");
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Prints Matched");
    delay(2000);
    lcd.clear();
    lcd.print("Processing....."); 
    delay(2000);
    lcd.clear();
    lcd.print("Finger Stored"); 
    lcd.setCursor(0,1);
    lcd.print("as ID: "); lcd.print(id);
    delay(2000);
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    Serial.println("Fingerprints did not match");
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Did not match");
    delay(2000);
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }

  Serial.print("ID "); Serial.println(id);
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    Serial.println("Stored!");
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("Finger Stored");
    lcd.setCursor(0,1);
    lcd.print("as ID: "); lcd.print(id);
    id =0;
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    Serial.println("Could not store in that location");
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    Serial.println("Error writing to flash");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }

  return true;
}

//Identify Function
uint8_t getFingerprintID() {
  lcd.clear();
  lcd.print("Place Finger....");
  uint8_t p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      lcd.clear();
      lcd.print("Finger Scanned....");
      break;
    case FINGERPRINT_NOFINGER:
      Serial.println("No finger detected");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK success!

  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Could not find fingerprint features");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK converted!
  p = finger.fingerSearch();
  if (p == FINGERPRINT_OK) {
    Serial.println("Found a print match!");
    lcd.clear();
    lcd.print("Finger found");
    lcd.setCursor(0,1);
    lcd.print("with ID: "); lcd.print(finger.fingerID);
    delay(3000);
    //voteContinue(finger.fingerID);

//    bool isVoted = searchInEEPROM(finger.fingerID);
//    if(isVoted == true){
//      alreadyVoted();
//      //return 0;
//    }
//    else{
//      votingProcess();
//      storingId(finger.fingerID);
//      successfullVote();
//
//    }
    idReturned++;
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    idReturned++;
    return p;
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("Did not find a match");
    idReturned++;
    return p;
  } else {
    Serial.println("Unknown error");
    idReturned++;
    return p;
  }
  
  // found a match!
  //returning the found fingerprint
  return finger.fingerID;
}

// returns -1 if failed, otherwise returns ID #
int getFingerprintIDez() {
  uint8_t p = finger.getImage();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.image2Tz();
  if (p != FINGERPRINT_OK)  return -1;

  p = finger.fingerFastSearch();
  if (p != FINGERPRINT_OK)  return -1;

  // found a match!
  
  return finger.fingerID;
}

int LED = D7;
int doorSensor = D0;

int status = 0;                 // 0:standby, 1:waiting, 2:swinging
int doorOpen = false;           // true when door is open
int prevDoorOpen = false;       // for remembering door state

int openPeriod = 0;             // tracks how long door is open
int openThreshold = 500;        // helps avoid false positive openings

int closedPeriod = 0;           // tracks how long door is closed
int closedThreshold = 5000;     // helps determine when door is done swinging

int stuckPeriod = 0;            // tracks how long door is stuck open
int stuckThreshold = 10000;     // how long before considering door is stuck
int sentStuckNotice = false;    // helps reduce too many notices

void setup(){
  pinMode(LED, OUTPUT);
  pinMode(doorSensor, INPUT_PULLUP);
  Particle.subscribe("hook-response/iotPetDoor", onWebhookResponse, MY_DEVICES);
  Serial.begin(9600);
}

void loop(){
  Serial.print(status);
  Serial.print(" ");

  if(status == 0 || status == 2){
    onStandby();
  }
  else if(status == 1){
    onPending();
  }
}

void onStandby(){
  checkDoorStatus();

  if(status == 0 && openPeriod >= openThreshold){
    sendMessage();
  }

  if(stuckPeriod >= stuckThreshold && sentStuckNotice == false){
    Serial.println("");
    Serial.println("_-_-_-_-_- BROKEN DOOR -_-_-_-_-_");
    Particle.publish("iotPetDoor", "The door is broken, you geez :/", PRIVATE);
    sentStuckNotice = true;
  }

  if(status == 2 && closedPeriod >= closedThreshold){
    status = 0;
    sentStuckNotice = false;
  }

  Serial.println("");
}

void onPending(){
  digitalWrite(LED, HIGH);
  delay(20);
  digitalWrite(LED, LOW);
  delay(100);
}

void sendMessage(){
  Serial.println("");
  Serial.println("Sending Message");
  Particle.publish("iotPetDoor", "I used my door, you geez :P", PRIVATE);
  status = 1;
}

void onWebhookResponse(const char *event, const char *data) {
  for(int a = 0; a < 5; a++){
    digitalWrite(LED, HIGH);
    delay(500);
    digitalWrite(LED, LOW);
    delay(100);
  }
  digitalWrite(LED, LOW);
  status = 2;
}

void checkDoorStatus(){
  doorOpen = digitalRead(doorSensor);

  if(doorOpen == prevDoorOpen){
    Serial.print("same status ");
    if(!doorOpen){
      closedPeriod++;
      Serial.print("closed (");
      Serial.print(closedPeriod);
      Serial.print(") ");
    }
    else {
      openPeriod++;
      Serial.print("open (");
      Serial.print(openPeriod);
      Serial.print(") ");

      if(openPeriod >= stuckThreshold){
        stuckPeriod++;
        Serial.print("stuck (");
        Serial.print(stuckPeriod);
        Serial.print(") ");
      }
    }
  }
  else {
    prevDoorOpen = doorOpen;
    Serial.print("now ");
    Serial.print(doorOpen ? "open " : "closed ");

    if(!doorOpen){
      Serial.print("reset open + stuck");
      openPeriod = 0;
      stuckPeriod = 0;
      sentStuckNotice = false;
    }
    else {
      Serial.print("reset closed");
      closedPeriod = 0;
    }
  }
}
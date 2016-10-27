int LED = D7;
int doorSensor = D0;

// 0: standby
// 1: waiting for response
// 2: waiting for swinging to stop
int status = 0;
int stuckThreshold = 5000;
int stuckCount = 0;
int sentStuckNotice = false;

int sendThreshold = 500;

int swingPeriod = 0;
int prevSwingPeriod = 0;

int calmPeriod = 0;
int calmThreshold = 5000;

int doorOpen = false;
int prevDoorOpen = false;

void setup(){
    pinMode(LED, OUTPUT);
    pinMode(doorSensor, INPUT_PULLUP);
    Particle.subscribe("hook-response/petDoor", onWebhookResponse, MY_DEVICES);
    Serial.begin(9600);
}

void loop(){
    if(status == 0 || status == 2){
        onStandby();
    }
    else if(status == 1){
        onPending();
    }
}

void onStandby(){
    // checking the door status advances any timers
    // that are currently being watched
    checkDoorStatus();

    // avoids false positives; only when door
    // is "officially" open, send the message
    if(status == 1 && swingPeriod >= sendThreshold){
        sendMessage();
    }

    // but if we've been stuck for a while and haven't told anyone,
    // send that notice out and put back into standby.
    if(stuckCount == 5 && sentStuckNotice == false){
        Serial.println("_-_-_-_-_- BROKEN DOOR -_-_-_-_-_");
        Particle.publish("petDoor", "The door is broken, you geez :/", PRIVATE);
        sentStuckNotice = true;
        status = 0;
    }
}

void onPending(){
    digitalWrite(LED, HIGH);
    delay(20);
    digitalWrite(LED, LOW);
    delay(100);
}

void sendMessage(){
    Serial.println("Sending Message");
    Particle.publish("petDoor", "I used my door, you geez :P", PRIVATE);
    status = 1;
}

void onWebhookResponse(const char *event, const char *data) {
    for(int a = 0; a < 5; a++){
        digitalWrite(LED, HIGH);
        delay(1000);
        digitalWrite(LED, LOW);
        delay(500);
    }
    digitalWrite(LED, LOW);
    status = 2;
}

void checkDoorStatus(){
    doorOpen = digitalRead(doorSensor);

    // when the door changes states (open, closed)
    if(doorOpen != prevDoorOpen){
        // remember this new state or this will only happen once
        prevDoorOpen = doorOpen;

        if(!doorOpen){
            // we want to remember how long the door has been opened
            // and reset the counter to prepare for the next swing.
            // reset stuck counter because door is not open.
            prevSwingPeriod = swingPeriod;
            swingPeriod = 0;
            stuckCount = 0;
        }

        // when the door closes, we want to time how long
        // this period of calm lasts so reset the counter.
        if(doorOpen){
            calmPeriod = 0;
        }
    }

    // when the door is just sitting there
    else {
        // when closed, track how long it's been closed
        if(!doorOpen){
            calmPeriod++;
        }

        // when open, track that time too
        else {
            swingPeriod++;

            // if the door has been open longer than expected
            // start to consider that it might be stuck.
            if(swingPeriod % stuckThreshold == 0){
                stuckCount++;
            }
        }

        // when the door has been closed for a while
        // reset any counters and put device into standby
        if(swingPeriod == 0 && calmPeriod >= calmThreshold){
            prevSwingPeriod = 0;
            status = 0;
        }
    }

    Serial.print("calm for: ");
    Serial.print(calmPeriod);
    Serial.print(" | open for: ");
    Serial.print(swingPeriod);
    Serial.print(" | stuck for: ");
    Serial.println(stuckCount);

}
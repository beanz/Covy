int sun_delay = 0;
long check_timer = 0;
long daylight_timer = 0;
time_store times[4];
long last_timezone_offset=-1;

void save_time(int i);

int command = -1;

bool MQTT_ON = false;
bool MQTT_SETUP = false;

String mqtt_device_name = "MorningRod";
String mqtt_server = "192.168.50.178";

String mqtt_username = "username";
String mqtt_password = "password";

//HA Auto Discovery
String discovery_prefix = "homeassistant";
String mqtt_discovery_topic = "homeassistant/cover/MorningRod/config";
String mqtt_state_topic =  "homeassistant/cover/MorningRod/state";
String mqtt_set_topic =  "homeassistant/cover/MorningRod/set";
String mqtt_discovery_payload;

BLYNK_WRITE(V15) { // MQTT
  if(param.asInt()!=0){
    DEBUG_STREAM.println("MQTT Turned ON");
    MQTT_ON = true; // tell control loop what to do
  }
  else{
    DEBUG_STREAM.println("MQTT Turned OFF");
    MQTT_ON = false;
    MQTT_SETUP = false;
  }
}

BLYNK_WRITE(V16) { //MQTT Device Name
  DEBUG_STREAM.print("Set MQTT Device Name: ");
  mqtt_device_name = param.asStr();
  mqtt_discovery_topic = discovery_prefix + "/cover/" + mqtt_device_name + "/config";
  mqtt_state_topic =  discovery_prefix + "/cover/" + mqtt_device_name + "/state";
  mqtt_set_topic =  discovery_prefix + "/cover/" + mqtt_device_name + "/set";
  preferences.putString("mqtt_device_name", mqtt_device_name);
  DEBUG_STREAM.println(mqtt_device_name);
}

BLYNK_WRITE(V17) { //IP Address
  DEBUG_STREAM.print("Set IP Address: ");
  mqtt_server = param.asStr();
  preferences.putString("mqtt_server", mqtt_server);
  DEBUG_STREAM.println(mqtt_server);
}

BLYNK_WRITE(V18) { //MQTT Username
  DEBUG_STREAM.print("Set MQTT Username: ");
  mqtt_username = param.asStr();
  preferences.putString("mqtt_username", mqtt_username);
  DEBUG_STREAM.println(mqtt_username);
}

BLYNK_WRITE(V19) { //MQTT Password
  DEBUG_STREAM.print("Set MQTT Password: "); 
  mqtt_password = param.asStr();
  preferences.putString("mqtt_password", mqtt_password);
  DEBUG_STREAM.println(mqtt_password);
}

BLYNK_WRITE(V20) { //Discovery Prefix
  DEBUG_STREAM.print("Set MQTT Topic 1: ");
  discovery_prefix = param.asStr();
  mqtt_discovery_topic = discovery_prefix + "/cover/" + mqtt_device_name + "/config";
  mqtt_state_topic = discovery_prefix + "/cover/" + mqtt_device_name + "/state";
  mqtt_set_topic = discovery_prefix + "/cover/" + mqtt_device_name + "/set";
  preferences.putString("discovery_prefix", discovery_prefix);
  DEBUG_STREAM.println(discovery_prefix);
}

WidgetRTC rtc;
BLYNK_CONNECTED() {
  // Synchronize time on connection
  rtc.begin();
  // Synchronize virtual pins on connection
  Blynk.syncAll();
  Blynk.virtualWrite(V27, Version);
}


BLYNK_WRITE(V64) { // sunrise/sunset delay
  sun_delay = param.asInt();
  preferences.putInt("sun_delay", sun_delay);
}

BLYNK_WRITE(V12) { // Move close now
  if(param.asInt()!=0){
    command = MOVE_CLOSE; // tell control loop what to do
  }
}

BLYNK_WRITE(V13) { // Move open now
  if(param.asInt()!=0){
    command = MOVE_OPEN; // tell control loop what to do
  }
}

BLYNK_WRITE(V5) { // open1
  times[0].active=param.asInt()!=0;
  preferences.putUChar("active_0", times[0].active);
}

BLYNK_WRITE(V7) { // close1
  times[1].active=param.asInt()!=0;
  preferences.putUChar("active_1", times[1].active);
}

BLYNK_WRITE(V9) { // open2
  times[2].active=param.asInt()!=0;
  preferences.putUChar("active_2", times[2].active);
}

BLYNK_WRITE(V11) { // close2
  times[3].active=param.asInt()!=0;
  preferences.putUChar("active_3", times[3].active);
}

BLYNK_WRITE(V122) { // set global velocity
  DEBUG_STREAM.print("set velocity: ");
  long q=param.asInt()*1000L;
  DEBUG_STREAM.println(q);
  preferences.putLong("velocity", q);
  velocity = q;
}

BLYNK_WRITE(V123) { // set stallguard OPEN value
  DEBUG_STREAM.print("set OPEN stall: ");
  int q=param.asInt();
  DEBUG_STREAM.println(q);
  if(q>63)q=63;
  if(q<-64)q=-64;
  q&=0x7F;
  q=q<<16;
  stall_open = q;
  preferences.putInt("stall_open", stall_open);
  DEBUG_STREAM.println(stall_open);
  DEBUG_STREAM.println(preferences.getInt("stall_open", stall_open));
}


BLYNK_WRITE(V124) { // set stallguard CLOSE value
  DEBUG_STREAM.print("set CLOSE stall: ");
  int q=param.asInt();
  DEBUG_STREAM.println(q);
  if(q>63)q=63;
  if(q<-64)q=-64;
  q&=0x7F;
  q=q<<16;
  stall_close = q;
  preferences.putInt("stall_close", stall_close);
  DEBUG_STREAM.println(preferences.getInt("stall_close", stall_close));
}

BLYNK_WRITE(V25) { // set Current OPEN value
  DEBUG_STREAM.print("set open current: ");
  int q=param.asInt();
  DEBUG_STREAM.println(q);
  double current = (255/(float)256)*((q+1)/(float)32)*(0.325/0.075)*(1/sqrt(2));
  Serial.print("Current: ");
  Serial.print(current);
  Serial.println(" Amps");
  Blynk.virtualWrite(V56, current);
  q&=0x1F;
  q=q<<8;
  current_open=q;
  preferences.putInt("current_open", current_open);
  DEBUG_STREAM.println(current_open);
}

BLYNK_WRITE(V26) { // set Current CLOSE value
  DEBUG_STREAM.print("set close current: ");
  int q=param.asInt();
  DEBUG_STREAM.println(q);
  double current = (255/(float)256)*((q+1)/(float)32)*(0.325/0.075)*(1/sqrt(2));
  Serial.print("Current: ");
  Serial.print(current);
  Serial.println(" Amps");
  Blynk.virtualWrite(V55, current);
  q&=0x1F;
  q=q<<8;
  current_close=q;
  preferences.putInt("current_close", current_close);
  DEBUG_STREAM.println(current_close);
  
}


BLYNK_WRITE(V22) { // set distance value
  DEBUG_STREAM.print("set distance: ");
  float q=param.asInt()*32492.59347; // One inch require this many microsteps
  preferences.putFloat("move_distance", q);
  move_distance = q;
  move_percent = q;
  DEBUG_STREAM.println(move_distance);
}

BLYNK_WRITE(V23) { // set percent open
  DEBUG_STREAM.print("set open percent to: ");
//  preferences.putFloat("move_percent", (move_distance/100)*param.asInt());
  float q=(move_distance/100)*param.asInt();
  preferences.putFloat("move_percent", q);
  move_percent = q;
  DEBUG_STREAM.println(move_percent);
}

BLYNK_WRITE(V4) {
  TimeInputParam t(param);

  // Process start time
  if (t.hasStartTime())
  {
    times[0].type=0;
    times[0].hour=t.getStartHour();
    times[0].minute=t.getStartMinute();
    times[0].second=t.getStartSecond();
  }
  else if (t.isStartSunrise())
  {
    times[0].type=1;
  }
  else if (t.isStartSunset())
  {
    times[0].type=2;
  }
  else
  {
    // Do nothing
  }
  for (int i = 1; i <= 7; i++) {
    times[0].day_sel[i-1]=t.isWeekdaySelected(i);
  }
  times[0].offset=t.getTZ_Offset();
  save_time(0);
}
BLYNK_WRITE(V6) {
  TimeInputParam t(param);

  // Process start time
  if (t.hasStartTime())
  {
    times[1].type=0;
    times[1].hour=t.getStartHour();
    times[1].minute=t.getStartMinute();
    times[1].second=t.getStartSecond();
  }
  else if (t.isStartSunrise())
  {
    times[1].type=1;
  }
  else if (t.isStartSunset())
  {
    times[1].type=2;
  }
  else
  {
    // Do nothing
  }
  for (int i = 1; i <= 7; i++) {
    times[1].day_sel[i-1]=t.isWeekdaySelected(i);
  }
  times[1].offset=t.getTZ_Offset();
  save_time(1);
}
BLYNK_WRITE(V8) {
  TimeInputParam t(param);

  // Process start time
  if (t.hasStartTime())
  {
    times[2].type=0;
    times[2].hour=t.getStartHour();
    times[2].minute=t.getStartMinute();
    times[2].second=t.getStartSecond();
  }
  else if (t.isStartSunrise())
  {
    times[2].type=1;
  }
  else if (t.isStartSunset())
  {
    times[2].type=2;
  }
  else
  {
    // Do nothing
  }
  for (int i = 1; i <= 7; i++) {
    times[2].day_sel[i-1]=t.isWeekdaySelected(i);
  }
  times[2].offset=t.getTZ_Offset();
  save_time(2);
}
BLYNK_WRITE(V10) {
  TimeInputParam t(param);

  // Process start time
  if (t.hasStartTime())
  {
    times[3].type=0;
    times[3].hour=t.getStartHour();
    times[3].minute=t.getStartMinute();
    times[3].second=t.getStartSecond();
  }
  else if (t.isStartSunrise())
  {
    times[3].type=1;
  }
  else if (t.isStartSunset())
  {
    times[3].type=2;
  }
  else
  {
    // Do nothing
  }
  for (int i = 1; i <= 7; i++) {
    times[3].day_sel[i-1]=t.isWeekdaySelected(i);
  }
  times[3].offset=t.getTZ_Offset();
  save_time(3);
}

BLYNK_WRITE(V35) { 
  if(param.asInt()!=0){
    Blynk.email("{DEVICE_NAME} : Token", configStore.cloudToken);
  }
}

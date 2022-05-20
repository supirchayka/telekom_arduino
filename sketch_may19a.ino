#include <SPI.h>
#include <Ethernet.h>

//Задаем нормера пинов для датчиков
#define temp_sensor_1_pin 4
#define temp_sensor_2_pin 5
#define temp_sensor_3_pin 6
#define temp_sensor_4_pin 8
#define temp_sensor_5_pin 9
#define temp_sensor_6_pin 10
#define temp_sensor_7_pin 12

//Задаем температуру тревоги
#define temp_alarm 30.0

//Задаем мак адрес нашего устройства
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

EthernetClient client;

int    HTTP_PORT   = 80;
char   HOST_NAME[] = "localhost";

//Создаем массив-перечисление для датчиков, что-бы упростить их перебор
byte sensors_in[] = { 
  temp_sensor_1_pin, 
  temp_sensor_2_pin, 
  temp_sensor_3_pin, 
  temp_sensor_4_pin,
  temp_sensor_5_pin, 
  temp_sensor_6_pin, 
  temp_sensor_7_pin, 
  };

//Создаем константу с колличеством датчиков
int const sensors_count = sizeof(sensors_in)/sizeof(*sensors_in);

//Объявляем класс датчика для универсальности и облегчения работы с ним
class Sensor {
  private:
    byte pin;
    float temp;
    String nameof; 
    
  public:
    Sensor(byte pin, int num){
      //Присваиваем датчику пин
      this->pin = pin;
      
      //Даем название датчику
      this->nameof = "Датчик температуры " + num;
    }

    //Функция для получения данных о температуре
    float getTemp(){
      this->temp = analogRead(this->pin) * 0.48828125;
      return this->temp;
    }

    //Функция для получения названия датчика
    String getName(){
      return this->nameof;
    }
    
};

//Функция формирования данных о всех датчиках
String preformSensorsData(String nameof, float temp, String data){
  return data + nameof + ": " + temp + "*C\n";
}

//Функция формирования данных о перегретых датчиках
String preformOverheatData(String nameof, String data){
  return nameof + ", " + data;
}

//Функция отправки данных о перегреве
bool sendOverheatToDatabase(String temperatures_data, String temperatures_high) {
  String state = "Зафиксировано превышение температуры";
  String description = temperatures_high + " превышение температуры";
  
  String queryString = "?state=" + state + "&" + 
  "description=" + description + "&" + 
  "temperatures=" + temperatures_data;

  String path_name = "/sendOverheat.php";
  String method = "POST";
  
  return sendQuerry(method, queryString, path_name);
}

//Универсальная функция отправки запроса на сервер
bool sendQuerry(String method, String queryString, String path_name){

  if (Ethernet.begin(mac) == 0) {
    Serial.println("Failed to obtaining an IP address using DHCP");
    return false;
  }

  //Соединяемся с сервером
  if(client.connect(HOST_NAME, HTTP_PORT)) {

    //Собираем заголовок запроса
    client.println(method + " " + path_name + " HTTP/1.1");
    client.println("Host: " + String(HOST_NAME));
    client.println("Connection: close");
    client.println();

    //Собираем тело запроса
    client.println(queryString);

    //Читаем ответ от сервера
    while(client.connected()) {
      if(client.available()){
        char c = client.read();
        Serial.print(c);
      }
    }

    //Отключаемся от сервера
    client.stop();
    Serial.println();
    Serial.println("disconnected");
  } else { //Если не удалось подключиться
    Serial.println("connection failed");
    return false;
  }

  return true;
}

//Объявляем массив размером == количство датчиков
Sensor *sensors[sensors_count];

//Запускаемся
void setup() {
  //Тут происходит магия
  Serial.begin(9600);

  //Создаем объект датчика и помещаем его в массив с объектами
  for(int i = 0; i < sensors_count; i++){
    sensors[i] = new Sensor(sensors_in[i], i + 1);
  }
  
}

void loop() {
  //Строка состояния всех датчиков
  String temperatures_data = "";

  //Строка для датчиков с повышенной температурой
  String temperatures_high = "";

  //Флажок для перегрева
  bool overheat = false;

  //Перебираем все датчики
  for(int i = 0; i < sensors_count; i++){
    
    String nameof = sensors[i]->getName();
    float temp = sensors[i]->getTemp();
    
    temperatures_data = preformSensorsData(nameof, temp, temperatures_data);

    //Проверяем больше ли температура чем температура тревоги
    if( temp >= temp_alarm ){
      temperatures_high = preformOverheatData(nameof, temperatures_high);
      overheat = true;
    }
  }

  //Если есть перегрев
  if (overheat){
    bool result = sendOverheatToDatabase(temperatures_data, temperatures_high);
    
    //Проверка результата отправки
    if(result){
      Serial.println("request fail");
    } 
    else{
      Serial.println("request success");
    }
  }
}

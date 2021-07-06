#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

#ifndef System_h
#define System_h

//Estrutura de callback da task
typedef void (*taskCallback)();

//Estrutura padrão de uma task (função a executar, tempo máximo, tempo atual)
typedef struct
{
	taskCallback callback;
	const int maxCounter;
	int downCounter;
} Task;  

//Array contendo todas as tasks do programa
extern Task tasks[];
//Bit para verificar se deve executar o callback da task
extern volatile unsigned long callTask;
//Quantidade de tasks
extern const int tasksCount;

//Variaveis global
//Estado do led1
extern bool led1State;

//Variaveis para o controle do bounce
extern int button1State;
extern int lastButton1State;
extern unsigned long lastButton1DebounceTime;

//Estado do led2
extern bool led2State;

//Variaveis para o controle do bounce
extern int button2State;
extern int lastButton2State;
extern unsigned long lastButton2DebounceTime;

//WebServer
extern AsyncWebServer server;
extern AsyncWebSocket ws;

//Variaveis do NTP
extern WiFiUDP ntpUDP;
extern NTPClient ntpClient;

//Flag para atualizar o horario
extern bool updateTimeFlag;
//Flag para coletar os dados do sensor
extern bool readSensorFlag;
//Flag para ler os botões
extern bool readButtonFlag;

#endif
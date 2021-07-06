#include "./Pins.h"
#include "./System.h"
#include "WebPage.h"
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoOTA.h>
#include <NTPClient.h>
#include <WiFiUdp.h>

//Variaveis global
//Estados dos leds
bool led1State;
bool led2State;

int button1State = LOW;
int lastButton1State = LOW;
unsigned long lastButton1DebounceTime = 0;

int button2State = LOW;
int lastButton2State = LOW;
unsigned long lastButton2DebounceTime = 0;

//Configuração do web server (porta 80)
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

//Configuração NTP
//Servidor NTP para pesquisar a hora
const char *serverNTP = "a.st1.ntp.br";
//Fuso horário em segundos (-03h = -10800 seg)
const int timezone = -10800;
//Declaração do Protocolo UDP
WiFiUDP ntpUDP;
NTPClient ntpClient(ntpUDP, serverNTP, timezone, 60000);

//Setup da serial
void InitSerial()
{
	Serial.begin(115200);
}

//Setup NTP
void InitNtp()
{
	ntpClient.begin();
	ntpClient.update();
}

//Setup do WebServer e do WebSocket
void InitWebServer()
{
	//Definindo a rota padrão para a pagina
	server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
	{
		request->send_P(200, "text/html", index_html, Processor);
	});

	ws.onEvent(OnEvent);
	server.addHandler(&ws);

	//Iniciando o servidor
	server.begin();
}

//Setup OTA
void InitOTA()
{
	ArduinoOTA.onStart([]() 
	{
		Serial.println("[OTA] Iniciado");
	});
	ArduinoOTA.onEnd([]()
	{
		Serial.println("[OTA] Finalizado");
	});
	ArduinoOTA.onProgress([](unsigned int progress, unsigned int total)
	{
		Serial.printf("\n[OTA] Progresso: %u%%\r", (progress / (total / 100))); 
	});
	ArduinoOTA.onError([](ota_error_t error)
	{
		Serial.printf("\n[OTA] Erro[%u]: ", error);
		if (error == OTA_AUTH_ERROR)
			Serial.println("[OTA] Falha na autenticaçao");
		else if (error == OTA_BEGIN_ERROR)
			Serial.println("[OTA] Falha ao iniciar");
		else if (error == OTA_CONNECT_ERROR)
			Serial.println("[OTA] Falha ao conectar");
		else if (error == OTA_RECEIVE_ERROR)
			Serial.println("[OTA] Falha ao receber os dados");
		else if (error == OTA_END_ERROR)
			Serial.println("[OTA] Falha ao finalizar");
	});
	ArduinoOTA.begin();
}

//Setup do arduino
void InitPinMode()
{
	//Leds
	pinMode(LED1PIN, OUTPUT);
	pinMode(LED2PIN, OUTPUT);
	
	//Botões
	pinMode(BUTTON1PIN, INPUT);
	pinMode(BUTTON2PIN, INPUT);
	
	//LDR
	pinMode(LDRPIN, INPUT);
}
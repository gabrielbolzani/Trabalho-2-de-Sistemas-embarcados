#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoOTA.h>
#include "WebPage.h"
#include "./System.h"
#include <NTPClient.h>
#include <WiFiUdp.h>

//Configuração da rede
const char* SSID = "LFFT";
const char* PASSWORD = "galodoido";

//Config do firmware
const String VERSION = "0.1.5";

bool ledState1 = LOW;
bool ledState2 = LOW;
const char ledPin1 = 2; //Porta Fisica Node MCU D4
const char ledPin2 = 4; //Porta Fisica Node MCU D2

#define botPin1 13 //Porta Fisica Node MCU D7
#define botPin2 15 //Porta Fisica Node MCU D8

//Configuração do web server (porta 80)
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

//Configuração NTP
//Servidor NTP para pesquisar a hora
const char *servidorNTP = "a.st1.ntp.br";
//Fuso horário em segundos (-03h = -10800 seg)
const int fusoHorario = -10800;
//Taxa de atualização do servidor NTP em milisegundos
const int taxaDeAtualizacao = 1800000;
//Declaração do Protocolo UDP
WiFiUDP ntpUDP;
NTPClient ntpClient(ntpUDP, servidorNTP, fusoHorario, 60000);

//Envia para todos os clientes o estado atual
void NotifyClients()
{
	ws.textAll(String(ledState1)+String(ledState2));
}

//Gerencia os comandos recebidos pelo web socket
void HandleWebSocketMessage(void *arg, uint8_t *data, size_t len)
{
	AwsFrameInfo *info = (AwsFrameInfo*)arg;
	if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
	{
		data[len] = 0;
		//Se o comando for toggle, muda o estado do led e notifica todos os clientes
		if (strcmp((char*)data, "toggle1") == 0)
		{
			ledState1 = !ledState1;
			NotifyClients();
		}
   if (strcmp((char*)data, "toggle2") == 0)
   {
      ledState2 = !ledState2;
      NotifyClients();
    }
	}
}

//Eventos do web socket
void OnEvent(AsyncWebSocket *server, AsyncWebSocketClient *client, AwsEventType type, void *arg, uint8_t *data, size_t len)
{
	switch (type)
	{
		case WS_EVT_CONNECT:
			Serial.printf("[WebSocket] Cliente #%u conectado %s\n", client->id(), client->remoteIP().toString().c_str());
			//Sempre que um cliente é conectado notifica o estado pra ele
			NotifyClients();
			break;
		case WS_EVT_DISCONNECT:
			Serial.printf("[WebSocket] Cliente #%u desconectado\n", client->id());
			break;
		case WS_EVT_DATA:
			HandleWebSocketMessage(arg, data, len);
			break;
		case WS_EVT_PONG:
			break;
		case WS_EVT_ERROR:
			break;
	}
}

//Setup do web socket
void InitWebSocket()
{
	ws.onEvent(OnEvent);
	server.addHandler(&ws);
}

//Setup NTP
void InitNtp()
{
	ntpClient.begin();
	ntpClient.update();
}

String Processor(const String& var)
{
	if(var == "STATE1")
	{
		if (ledState1)
			return "ON";
		else
			return "OFF";
	}
  else if(var == "STATE2")
  {
    if (ledState2)
      return "ON";
    else
      return "OFF";    
  }
	else if(var == "TIME")
		return ntpClient.getFormattedTime().c_str();
	else if(var == "VERSION")
		return VERSION;
	else if(var == "RSSI")
		return String(WiFi.RSSI());
	else if(var == "IP")
		return WiFi.localIP().toString();
	else if(var == "MAC")
		return WiFi.macAddress().c_str();
}

void setup()
{
	//Setup da serial
	InitSerial();
  
	//Setup ESP
	pinMode(botPin1, INPUT);
  pinMode(botPin2, INPUT);
  
	pinMode(ledPin1, OUTPUT);
  pinMode(ledPin2, OUTPUT);
  
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

	//Conecta na rede
	WiFi.begin(SSID, PASSWORD);
	while (WiFi.status() != WL_CONNECTED)
	{
		delay(1000);
		Serial.println("[ESP] Conectando a rede wifi");
	}

	Serial.println("[ESP] IP Adquirido:");
	Serial.println(WiFi.localIP());

	//Setup do web socket
	InitWebSocket();

	//Definindo a rota padrão para a pagina
	server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
	{
		request->send_P(200, "text/html", index_html, Processor);
	});

	//Iniciando o servidor
	server.begin();
	
	//Executa o setup dos pinos
	InitPinMode();
	
	//Inicia o contador das tasks
	InitCountdown();
	
	//Inicia o NTP
	InitNtp();
}

void loop()
{
	//Gerenciando o OTA
	ArduinoOTA.handle();

	//Limpa os clientes
	ws.cleanupClients();
	if(digitalRead(botPin1)==1){
    ledState1 = !ledState1;
    delay(100);
	}
  if(digitalRead(botPin2)==1){
    ledState2 = !ledState2;
    delay(100);
  }
	//Atualiza o led
	digitalWrite(ledPin1, !ledState1);
  digitalWrite(ledPin2, !ledState2);	
	//Executa o gerenciamento das tasks
	HandleTasks();
}

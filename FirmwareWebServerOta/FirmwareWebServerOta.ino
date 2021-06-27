#include <ESP8266WiFi.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <ArduinoOTA.h>
#include "WebPage.h"

#include <NTPClient.h>
#include <WiFiUdp.h>

const String VERSION = "0.1.4";
String TIME="NULL"; 

//Configuração da rede
const char* ssid = "GABRIEL";
const char* password = "99514334";

bool ledState = LOW;
const char ledPin = 2;

const char *servidorNTP = "a.st1.ntp.br"; // Servidor NTP para pesquisar a hora
const int fusoHorario = -10800;           // Fuso horário em segundos (-03h = -10800 seg)
const int taxaDeAtualizacao = 1800000;    // Taxa de atualização do servidor NTP em milisegundos
WiFiUDP ntpUDP; // Declaração do Protocolo UDP
NTPClient timeClient(ntpUDP, servidorNTP, fusoHorario, 60000);

//Configuração do web server (porta 80)
AsyncWebServer server(80);
AsyncWebSocket ws("/ws");

//Envia para todos os clientes o estado atual
void NotifyClients()
{
	ws.textAll(String(ledState));
}

//Gerencia os comandos recebidos pelo web socket
void HandleWebSocketMessage(void *arg, uint8_t *data, size_t len)
{
	AwsFrameInfo *info = (AwsFrameInfo*)arg;
	if (info->final && info->index == 0 && info->len == len && info->opcode == WS_TEXT)
	{
		data[len] = 0;
		//Se o comando for toggle, muda o estado do led e notifica todos os clientes
		if (strcmp((char*)data, "toggle") == 0)
		{
			ledState = !ledState;
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

String Processor(const String& var){
  Serial.println(var);
  if(var == "STATE"){
    if (ledState){
      return "ON";
    }
    else{
      return "OFF";
    }
  }
  if(var == "TIME"){
    return timeClient.getFormattedTime().c_str();
  }
  if(var == "VERSION"){
    return VERSION;
  }
  if(var == "RSSI"){
    return String(WiFi.RSSI());
  }
  if(var == "IP"){
    return WiFi.localIP().toString();
  }
  if(var == "MAC"){
    return WiFi.macAddress().c_str();
  }
}

void setup()
{
  //WiFi.setHostname("ESP8266");
	Serial.begin(115200);
  
	//Setup ESP
	pinMode(ledPin, OUTPUT);
  
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
	WiFi.begin(ssid, password);
	while (WiFi.status() != WL_CONNECTED)
	{
		delay(1000);
		Serial.println("[ESP] Conectando a rede wifi");
	}

	Serial.println("[ESP] IP Adquirido:");
	Serial.println(WiFi.localIP());

  timeClient.begin();
  timeClient.update();
  Serial.print("Horario atualizado: ");
  Serial.println(timeClient.getFormattedTime());
	//Setup do web socket
	InitWebSocket();

	//Definindo a rota padrão para a pagina
	server.on("/", HTTP_GET, [](AsyncWebServerRequest *request)
	{
		request->send_P(200, "text/html", index_html, Processor);
	});

	//Iniciando o servidor
	server.begin();
}

void loop()
{
	//Gerenciando o OTA
	ArduinoOTA.handle();

	//Limpa os clientes
	ws.cleanupClients();
	
	//Atualiza o led
	digitalWrite(ledPin, !ledState);
}

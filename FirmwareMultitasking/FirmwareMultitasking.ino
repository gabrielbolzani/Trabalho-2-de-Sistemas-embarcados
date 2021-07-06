#include "./System.h"
#include "Pins.h"
#include "WifiConfig.h"
#include <ArduinoOTA.h>

void setup()
{
	//Executa os setups
	InitSerial();
	InitPinMode();
	
	//Conecta o ESP na rede
	WiFi.begin(SSID, PASSWORD);
	while (WiFi.status() != WL_CONNECTED)
	{
		delay(1000);
		Serial.println("[ESP] Conectando a rede wifi");
	}

	Serial.println("[ESP] IP Adquirido:");
	Serial.println(WiFi.localIP());
	
	//Executa os setups
	InitOTA();
	InitNtp();
	InitWebServer();
	InitCountdown();
}

void loop()
{
	//Gerenciando o OTA
	ArduinoOTA.handle();
	
	//Limpa os clientes
	ws.cleanupClients();

	//Atualiza o ntp
	ntpClient.update();
	
	//Executa o gerenciamento das tasks
	HandleTasks();
	
	//Atualiza os leds
	digitalWrite(LED1PIN, led1State);
	digitalWrite(LED2PIN, led2State);
	
	//Checa a flag de atualizar o horario
	if(updateTimeFlag)
	{
		//Notifica o novo tempo baseado no NTP
		NotifyTime();
		
		//Reseta a flag
		updateTimeFlag = false;
	}
	
	//Checa a flag de coletar os dados
	if(readSensorFlag)
	{
		//Coleta o dado do sensor e notifica o novo valor
		int ldr = analogRead(LDRPIN);
		NotifySensor(ldr);
		
		//Reseta a flag
		readSensorFlag = false;
	}
	
	//Checa a flag para ler os botões
	if(readButtonFlag)
	{
		//Lógica para controle do bounce e leitura do botão 1
		int button1Reading = digitalRead(BUTTON1PIN);
		
		if (button1Reading != lastButton1State)
			lastButton1DebounceTime = millis();
		
		if ((millis() - lastButton1DebounceTime) > 50)
		{
			if (button1Reading != button1State)
			{
				button1State = button1Reading;

				if (button1State == HIGH)
				{
					//Inverte o estado do led e notifica a mudança
					led1State = !led1State;
					NotifyLed1State();
				}
			}
		}
		
		lastButton1State = button1Reading;
		
		//Lógica para controle do bounce e leitura do botão 2
		int button2Reading = digitalRead(BUTTON2PIN);
		
		if (button2Reading != lastButton2State)
			lastButton2DebounceTime = millis();
		
		if ((millis() - lastButton2DebounceTime) > 50)
		{
			if (button2Reading != button2State)
			{
				button2State = button2Reading;

				if (button2State == HIGH)
				{
					//Inverte o estado do led e notifica a mudança
					led2State = !led2State;
					NotifyLed2State();
				}
			}
		}
		
		lastButton2State = button2Reading;
		
		//Reseta a flag
		readButtonFlag = false;
	}
}
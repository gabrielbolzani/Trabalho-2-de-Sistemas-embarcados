#include "./Pins.h"
#include "./System.h"

//Array com as tasks do sistema
Task tasks[] =
{
	//Task para atualizar o horario em todos os clientes, executa a cada 1s
	{SetTimeFlag, 1000, 1000},
	//Task para coletar os dados do sensor, executa a cada 500ms
	{ReadSensorData, 500, 500},
	//Task para ler os botões, executa a cada 10ms
	{ReadButton, 10, 10}
};

//Quantidade de tasks
const int tasksCount = (sizeof(tasks)) / sizeof(tasks[0]);

//Flag para atualizar o time dos clientes
bool updateTimeFlag = false;
void SetTimeFlag()
{
	updateTimeFlag = true;
	return;
}

//Flag para coletar os dados do sensor
bool readSensorFlag = false;
void ReadSensorData()
{
	readSensorFlag = true;
	return;
}

//Flar para ler o estado dos botões
bool readButtonFlag = false;
void ReadButton()
{
	readButtonFlag = true;
	return;
}

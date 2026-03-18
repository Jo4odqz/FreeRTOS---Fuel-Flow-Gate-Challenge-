# FreeRTOS---Fuel-Flow-Gate-Challenge-

## Descrição do Projeto
Implementação em C com FreeRTOS no ESP32 para simular um truque de sincronização em sensores de amostragem, baseada no caso da Fórmula 1. Demonstra uso de semáforos, tasks multicore e controle de delays.

## Requisitos de Hardware
O projeto foi desenvolvido para a plataforma ESP32 suportando FreeRTOS. A pinagem configurada no código é a seguinte:

* **LED Verde (GPIO 12):** Status OK (Leitura ≤ 100 unidades).
* **LED Vermelho (GPIO 13):** Violação detectada (Leitura > 100 unidades).
* **LED Indicador (GPIO 14):** Modo "Cheat" ativado (Cheat mode on/off).
* **Botão 1 (GPIO 27):** Switch A - Ativar "Cheat".
* **Botão 2 (GPIO 26):** Switch B - Forçar Falha (Induzir Falha).

## Arquitetura das Tarefas
O sistema foi dividido em duas tarefas principais distribuídas nos núcleos do processador:

* **Sensor (Tarefa de Monitoramento):** Tarefa de alta prioridade (Prioridade 3). Possui uma frequência de amostragem constante de 2000 vezes por minuto (aprox. 33,33 Hz ou a cada 30ms). A cada leitura, o sensor verifica o valor atual da "Injeção de Combustível".
* **Atuador (Tarefa de Injeção):** Simula a injeção de combustível (Prioridade 2) e opera em três modos controlados pelos botões:
  1. **Modo Legal (Padrão):** O fluxo de combustível é constante em 100 unidades.
  2. **Modo "Cheat" (Burlar):** O sistema aumenta o fluxo para 120 unidades, mas sincroniza com a tarefa do sensor para que, exatamente no momento da leitura, o fluxo caia para 100 unidades.
  3. **Modo "Falha Proposital":** O sistema tenta burlar, mas introduz um atraso proposital na sincronização, fazendo com que o sensor capture o valor de 120 unidades (para testar a detecção).

## Estratégia de Sincronização
Para garantir que a tarefa do atuador saiba exatamente quando o sensor vai realizar a leitura, utilizamos a abordagem de sincronização por **Semáforo Binário** (`sensor_ready`).

A `SensorTask` dita o ritmo do sistema usando `vTaskDelayUntil` para garantir a precisão temporal exigida e evitar as variações de um delay comum. Imediatamente antes de efetuar a leitura, ela libera o semáforo. 

A `InjectorTask` fica bloqueada aguardando a liberação desse semáforo. Quando ativada no modo *cheat*, ela eleva o combustível para 120, aguarda 29ms e reduz rapidamente para 100 pouco antes de o sensor acordar no milissegundo 30, enganando perfeitamente a fiscalização. Caso a falha seja induzida pelos switches, um atraso de 5ms extra é inserido, impedindo a redução a tempo e acionando a detecção da fraude (LED Vermelho).

## Visualização do Circuito


## Vídeo de Demonstração
Abaixo está a demonstração prática do sistema em funcionamento. O vídeo mostra o LED Verde operando normalmente mesmo quando o "cheat" está ativo, e o LED Vermelho acendendo apenas quando a falha na sincronização é induzida.
 
🔗 **[LINK VÍDEO DO YOUTUBE]**

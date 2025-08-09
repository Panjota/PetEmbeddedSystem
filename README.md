# PetEmbeddedSystem 🐾

Sistema automatizado de alimentação e hidratação para pets utilizando ESP32, sensores ultrassônicos, servo motor e bomba d'água, integrado com MQTT, Node-RED, Firebase Realtime Database e bot do Telegram para controle e monitoramento remoto.

## 📋 Descrição

Este projeto implementa um sistema completo de cuidado automatizado para pets, composto por dois módulos principais:

- **Módulo de Ração**: Dispensa ração automaticamente através de servo motor
- **Módulo de Água**: Controla o fornecimento de água através de bomba d'água
- **Sistema de Notificações**: Bot do Telegram para controle remoto e alertas
- **Banco de Dados**: Firebase Realtime Database para armazenamento de dados e histórico

Ambos os módulos são controlados remotamente via MQTT, monitorados através de uma interface Node-RED e integrados com bot do Telegram para controle e notificações em tempo real.

## 🔧 Componentes Utilizados

### Hardware
- 2x ESP32 (um para cada módulo)
- 4x Sensores ultrassônicos HC-SR04 (2 por módulo)
- 1x Servo motor (módulo ração)
- 1x Bomba d'água (módulo água)
- 2x Módulos relé
- 1x Sensor de nível resistivo (tigela de água)
- 1x Buzzer
- Jumpers e protoboard

### Software
- Arduino IDE
- Node-RED
- Broker MQTT (test.mosquitto.org)
- Firebase Realtime Database
- Bot do Telegram
- ngrok (para acesso remoto ao dashboard)

## 🚀 Funcionalidades

### Módulo de Ração (sketch_racao.ino)
- ✅ Dispensação automática de ração via comando MQTT
- ✅ Monitoramento do nível do reservatório de ração
- ✅ Detecção de presença do pet
- ✅ Alertas de nível baixo de ração
- ✅ Controle alternado do servo motor para distribuição uniforme

### Módulo de Água (sketch_agua.ino)
- ✅ Dispensação automática de água via comando MQTT
- ✅ Monitoramento do nível do reservatório de água
- ✅ Monitoramento do nível da tigela de água
- ✅ Detecção de visitas do pet à tigela
- ✅ Alertas de níveis baixos
- ✅ Notificação sonora (buzzer) ao dispensar água

### Bot do Telegram (PetBot)
- ✅ Controle remoto via comandos de texto
- ✅ Consulta de status dos reservatórios em tempo real
- ✅ Dispensação manual de ração e água
- ✅ Notificações automáticas de alertas
- ✅ Histórico de porções e visitas
- ✅ Interface amigável com botões interativos

### Funcionalidades Firebase
- 📊 **Armazenamento em tempo real** de dados dos sensores
- 📈 **Histórico ** de dispensações programadas e visitas

## 🤖 Bot do Telegram - PetBot

O bot do Telegram oferece controle completo do sistema através de uma interface intuitiva:

### Comandos Disponíveis
| Comando | Função |
|---------|--------|
| `/start` | Inicia o bot e mostra menu principal |
| **🥘 Nível Reservatório (Ração)** | Consulta nível atual do reservatório de ração |
| **🥘 Distância do Pet (Ração)** | Verifica proximidade do pet ao comedouro |
| **🥘 Liberar Ração** | Dispensa ração manualmente |
| **🍽️ Horários das Porções** | Visualiza Horário de alimentação |
| **💧 Nível Pote (Água)** | Consulta nível da tigela de água |
| **💧 Distância do Pet (Água)** | Verifica proximidade do pet ao bebedouro |
| **💧 Estado Pote (Água)** | Status detalhado da tigela (Vazio/Baixo/Médio/Alto) |
| **💧 Nível Reservatório (Água)** | Consulta nível do reservatório de água |
| **💧 Visitas ao Bebedouro** | Histórico de visitas do pet |
| **💧 Ligar Bomba** | Dispensa água manualmente |

### Recursos do Bot
- 🎯 **Interface intuitiva** com botões interativos
- 📊 **Consultas em tempo real** diretamente do Firebase
- 🔔 **Notificações push** para alertas importantes  
- 🎛️ **Controle total** dos dispositivos remotamente

## 🛠️ Configuração

### 1. Configuração do Hardware
1. Monte os circuitos conforme a pinagem especificada
2. Conecte os sensores ultrassônicos nos reservatórios e área do pet
3. Instale o servo motor no dispensador de ração
4. Conecte a bomba d'água ao sistema de água

### 2. Configuração do Software
1. Configure as credenciais WiFi nos arquivos .ino:
```cpp
const char* ssid = "SEU_WIFI";
const char* password = "SUA_SENHA";
```

2. Configure o broker MQTT (opcional - já está configurado para test.mosquitto.org):
```cpp
const char* mqttServer = "test.mosquitto.org";
const int mqttPort = 1883;
```

3. Carregue os sketches nos respectivos ESP32

### 3. Configuração do Firebase
1. Crie um projeto no [Firebase Console](https://console.firebase.google.com/)
2. Obtenha a URL do database e configure no Node-RED

### 4. Configuração do Bot Telegram
1. Crie um bot com o [@BotFather](https://t.me/BotFather)
2. Obtenha o token do bot
5. Conecte o bot ao Node-RED para consultas em tempo real

### 5. Configuração do Node-RED
1. Importe os fluxo
2. Configure os nós MQTT para conectar ao mesmo broker
3. Configure a integração com Firebase Realtime Database
4. Configure o bot do Telegram com o token obtido
5. Ajuste os tópicos e comandos conforme necessário

### 6. Configuração do ngrok (Acesso Remoto)
1. Instale o [ngrok](https://ngrok.com/) em seu sistema
2. Execute o Node-RED localmente (porta padrão 1880)
3. Crie um túnel HTTP para o Node-RED:
```bash
ngrok http 1880
```
4. Copie a URL pública gerada (ex: https://xyz123.ngrok-free.app)
5. Acesse o dashboard remotamente através da URL do ngrok
6. Configure autenticação no Node-RED para maior segurança

**Nota**: O ngrok permite acesso ao dashboard Node-RED de qualquer lugar com internet, mantendo o servidor local protegido.

#### Acesso Local e Remoto
- **Local**: `http://localhost:1880/ui` (na mesma rede)
- **Remoto**: Via ngrok para acesso de qualquer lugar (ex: `https://xyz123.ngrok-free.app/ui`)

As interfaces permitem:
- Dispensação manual remota
- Visualização em tempo real dos sensores
- Histórico de alertas
- Controle de horários programados
- **Acesso remoto seguro** através do túnel ngrok
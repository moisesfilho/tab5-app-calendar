# tab5-app-calendar

Aplicativo de Calendário Mensal para o sistema operacional **Tab5 OS** (M5Stack Tab5 / ESP32-P4), desacoplado e compilado para execução isolada em WebAssembly (WAMR).

## Características

- Visualização mensal em grade com suporte a navegação entre meses
- Destaque automático do dia atual obtido via relógio de alta precisão/RTC do Tab5 OS
- Interface responsiva com suporte a temas e rotação de tela
- Totalmente desacoplado do kernel do sistema operacional

## Como Compilar e Gerar o Pacote

```bash
# Executa o script de empacotamento
./tools/build.sh
```

O pacote resultante `com.tab5.calendar.tab5pkg` será criado na pasta `dist/` e pode ser instalado diretamente no dispositivo via Cartão SD (`/sdcard/apps/`) ou através do aplicativo Gerenciador de Armazenamento.

## Licença

MIT License.

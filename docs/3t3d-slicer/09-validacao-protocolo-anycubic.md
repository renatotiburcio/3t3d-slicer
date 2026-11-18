# Validação do protocolo LAN Anycubic

Em 2026-11-18, os testes isolados do adaptador comunitário localizado em `.tools/anycubic-orca-plugin` foram executados com `uv run pytest -q`.

Resultado: **34 testes passaram**.

O conjunto valida a identidade do agente, registro de callbacks, fluxo de autenticação `/info` + `/ctrl`, descoberta SSDP, payload entregue ao OrcaSlicer, detecção de dispositivo e tradução de estados.

Esta validação é simulada e não representa ainda uma conexão com uma Anycubic Kobra 4 Combo real. A habilitação do agente no 3T3D Slicer continua condicionada ao teste em rede local com a impressora ligada, para confirmar o modelo, autenticação, telemetria e compatibilidade do firmware.

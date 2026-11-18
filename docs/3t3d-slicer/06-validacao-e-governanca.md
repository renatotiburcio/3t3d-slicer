# Validação e governança

## Rotina mínima por mudança

1. `git status --short` e `git diff --stat` para identificar alterações preexistentes.
2. Validar JSON e referências de presets com o `OrcaSlicer_profile_validator`.
3. Configurar/buildar Release conforme `AGENTS.md`.
4. Executar CTest e registrar o número real de testes, não apenas o código de saída.
5. Rodar smoke test do executável em perfil de usuário temporário.
6. Para GUI, testar primeiro boot, troca de impressora, troca de filamento, fatiamento, exportação e reabertura.
7. Atualizar a matriz e registrar comando, data, commit e resultado.

## Validações específicas

### Perfis

- cada `compatible_printers` deve existir no bundle correspondente;
- nome de máquina, nozzle e processo devem coincidir exatamente;
- modelos/texturas de mesa devem ser empacotados;
- versões de fornecedor devem ser incrementadas quando exigido;
- perfis antigos devem continuar carregando ou ter migração documentada.

### Formatos e G-code

- round-trip `.3mf` em arquivo criado por versão anterior;
- comparação de G-code em casos de referência;
- teste com opção nova desligada para garantir comportamento anterior;
- inspeção de comandos específicos de fabricante antes de enviar à máquina.

### Rede e dispositivos

- nenhum segredo em logs ou documentação;
- TLS/verificação de certificado quando houver conexão;
- timeout, cancelamento e retry definidos;
- modo offline continua funcionando;
- testes com impressora indisponível, resposta inválida e firmware incompatível.

## Registro da fotografia atual

| Verificação | Resultado em 2026-11-17 |
|---|---|
| Git/status | modificações locais e arquivos não rastreados presentes |
| CMake cache | Visual Studio 18 2026, Release |
| Artefatos Release | executável, DLL e validador presentes |
| Smoke test do executável | `orca-slicer.exe` iniciou e permaneceu ativo por 8 s com os recursos atuais; encerrado ao final do teste |
| Rebuild após atualização visual | concluído com sucesso usando CMake 4.4.4/MSBuild 18.11.1; warnings de link/runtime não bloquearam os artefatos |
| Validador Anycubic | falhou por referências `addnorth` a impressoras Bambu desconhecidas |
| CTest discovery | `Total Tests: 0` |
| Execução gráfica | não comprovada de forma reproduzível nesta auditoria |

## Regra de documentação contínua

Não apagar histórico de sessão. Em vez disso, registrar nesta pasta o estado confirmado e apontar para a sessão como contexto. Ao concluir um item, trocar o estado somente após anexar a evidência. Ao descobrir uma regressão, reabrir o item na matriz e no roadmap.

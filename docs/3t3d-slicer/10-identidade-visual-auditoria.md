# Auditoria de identidade visual 3T3D

## Atualizado em 2026-11-18

Esta etapa foi executada sem abrir descoberta de impressoras, sem conexao LAN e sem alterar a impressao em andamento.

### Ajustes realizados

- O texto visivel da janela Sobre foi atualizado para 3T3D Slicer.
- A referencia de copyright da janela Sobre foi atualizada para 3T3D Slicer.
- O link antigo do site Orca foi removido da janela Sobre.
- O identificador exibido na barra de tarefas do macOS foi atualizado para 3T3D Slicer.
- O titulo da janela quando existe um projeto aberto agora usa `3T3D Slicer`.
- Os nomes internos de arquivos, targets, namespaces e chaves de compatibilidade foram preservados para evitar regressao em perfis, projetos 3MF e instalacoes existentes.

### Pendencias de auditoria

- Revisar somente textos visiveis restantes que citam Orca, Bambu Cloud, Wiki ou Design; referencias internas necessarias para compatibilidade nao devem ser removidas.
- Revisar os caminhos de recursos de icones e PNGs no pacote final, mantendo aliases internos quando necessario.
- Corrigir o warning de link `LNK4098` na origem da configuracao de runtime, sem mascarar incompatibilidade de CRT por supressao cega.
- Validar a identidade visual em uma compilacao Release e no executavel instalado.

## Validacao desta etapa

O alvo `OrcaSlicer_app_gui.vcxproj` foi compilado em Release com sucesso em 2026-11-18. O executavel foi gerado em `build/src/Release/orca-slicer.exe`.

O `LNK4098` foi rastreado às bibliotecas estáticas do libjpeg-turbo (`jpeg-static.lib`, `turbojpeg-static.lib` e componentes internos), que introduziam `LIBCMT` enquanto o aplicativo usa `/MD`. `deps/JPEG/JPEG.cmake` agora habilita `WITH_CRT_DLL=ON` e passa `/MD` em Release/RelWithDebInfo e `/MDd` em Debug. O CMake 4.4.4 regenerou o projeto, a dependência foi recompilada e `dumpbin` confirmou `MSVCRT` nas bibliotecas instaladas. O alvo Release foi relinkado com sucesso sem emitir `LNK4098`; esta limitação está resolvida.

### Limite desta etapa

Nenhuma descoberta Anycubic, conexao com cloud ou comando de controle foi executado enquanto a Kobra 4 Combo estava imprimindo.

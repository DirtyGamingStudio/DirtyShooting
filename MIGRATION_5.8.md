# Migração experimental para Unreal Engine 5.8.2

Origem preservada: C:\Users\bpasc\OneDrive\Desktop\Pasta do Projeto\ProjectUnreal
Cópia: C:\UnrealProjects\DirtyShooting_UE5.8

Copiados conteúdo, configuração, código-fonte, Build e plugin ShooterSettings. Excluídos .git, .vs, Binaries, Intermediate, DerivedDataCache, Saved, solução antiga e mt.exe. Renders intermediários e saves em Saved continuam no original. O vídeo final em Content foi copiado.

Validação da cópia: 1868 arquivos de Content idênticos por SHA256. Robocopy sem falhas.

| Arquivo na cópia | Alteração |
|---|---|
| ShooterGame.uproject | EngineAssociation 5.4 → 5.8 |
| Source/*.Target.cs | DefaultBuildSettings V5 → V7, exigido pela compilação com a engine instalada |
| Plugins/ShooterSettings/Source/ShooterSettings/Private/ShooterSettings.cpp | Tipo explícito APawn* no resultado de GetPawn(), para compatibilidade com TObjectPtr na 5.8 |

Logs de cópia e compilação em C:\UnrealProjects. Nenhuma alteração intencional no projeto original.

Resultado: build ShooterGameEditor Win64 Development SUCCEEDED na 5.8.2. Editor abriu UI_MainMenu e executou o menu em PIE, incluindo o vídeo de fundo. PIE encerrado; editor deixado aberto na cópia.

Ajuste adicional: Config/DefaultGame.ini, bAddPacks=False para evitar tentativa de importar StarterContent.upack ausente na instalação 5.8. Nenhum asset do jogo removido.

Limites: gameplay completo, empacotamento e performance ainda não validados. O teste registrou avisos de referência ausente a IMC_MouseLook no plugin, também observados em inspeções anteriores na 5.4; requer revisão separada antes de considerar esta cópia pronta para release.

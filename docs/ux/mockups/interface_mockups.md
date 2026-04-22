# Mockups de Interface - Sistema de Matricula AEDS III

## Mockup 1: Tela Principal (MainMenu)

```
┌─────────────────────────────────────────────────────────────────┐
│ ▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓ │
│                     AEDS III - Sistema de Matricula            │
│ ▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓▓ │
├────────────┬────────────────────────────────────────────────────┤
│            │                                                     │
│  ████ MENU ████│      Bem-vindo ao Sistema de Matricula        │
│            │                                                     │
│  [ Home ]  │      ┌────────────────────────────────────┐        │
│  [ Listar] │      │                                    │        │
│  [ Criar ] │      │    [═══════════════════════════]   │        │
│            │      │    [  CADASTRAR ESTUDANTE    ]   │        │
│            │      │    [═══════════════════════════]   │        │
│            │      │                                    │        │
│            │      │    [═══════════════════════════]   │        │
│            │      │    [   LISTAR ESTUDANTES    ]   │        │
│            │      │    [═══════════════════════════]   │        │
│            │      │                                    │        │
│            │      └────────────────────────────────────┘        │
│            │                                                     │
├────────────┴────────────────────────────────────────────────────┤
│ Status: 0 registros ativos                                      │
└─────────────────────────────────────────────────────────────────┘

Legenda: [████] = Botao Verde (Primario)   [      ] = Botao Preto (Secundario)
```

---

## Mockup 2: Cadastro de Estudante (StudentCreate)

```
┌─────────────────────────────────────────────────────────────────┐
│                                                                 │
│                     CADASTRAR ESTUDANTE           [VOLTAR]     │
│                                                                 │
├────────────┬────────────────────────────────────────────────────┤
│            │                                                     │
│  ████ MENU ████│  ┌─────────────────────────────────────────┐  │
│            │  │  │ Nome:                                     │  │
│  [ Home ]  │  │  │ [____________________________________]   │  │
│  [ Listar] │  │  │                                         │  │
│  [ Criar ] │  │  │ UserID:                                  │  │
│            │  │  │ [________]                               │  │
│            │  │  │                                         │  │
│            │  │  │ Data Nascimento (YYYYMMDD):              │  │
│            │  │  │ [________]                               │  │
│            │  │  │                                         │  │
│            │  │  │   [═══════════]  [═══════════]          │  │
│            │  │  │   [ CADASTRAR ]  [ CANCELAR ]           │  │
│            │  │  │    (Verde)       (Vermelho)             │  │
│            │  │  └─────────────────────────────────────────┘  │
│            │                                                     │
├────────────┴────────────────────────────────────────────────────┤
│ Status: Preencha todos os campos                                │
└─────────────────────────────────────────────────────────────────┘
```

---

## Mockup 3: Lista de Estudantes (StudentList)

```
┌─────────────────────────────────────────────────────────────────┐
│                                                                 │
│                     LISTA DE ESTUDANTES          [+ NOVO]     │
│                                                                 │
├────────────┬────────────────────────────────────────────────────┤
│            │                                                     │
│  ████ MENU ████│  ┌─────────────────────────────────────────┐  │
│            │  │  │ ID │ Nome                  │ Acoes        │  │
│  [ Home ]  │  │  │────┼──────────────────────┼───────────────│  │
│  [ Listar] │  │  │  1 │ Joao Silva           │ [E] [X]       │  │
│  [ Criar ] │  │  │  2 │ Maria Santos         │ [E] [X]       │  │
│            │  │  │  3 │ Pedro Costa          │ [E] [X]       │  │
│            │  │  │  4 │ Ana Oliveira         │ [E] [X]       │  │
│            │  │  │  5 │ Carlos Souza         │ [E] [X]       │  │
│            │  │  │    │                      │               │  │
│            │  │  └─────────────────────────────────────────┘  │
│            │                                                     │
│            │    Legenda: [E] = Editar  [X] = Excluir           │
│            │                                                     │
├────────────┴────────────────────────────────────────────────────┤
│ Total: 5 estudantes ativos                                       │
└─────────────────────────────────────────────────────────────────┘
```

---

## Mockup 4: Detalhes do Estudante (StudentDetail)

```
┌─────────────────────────────────────────────────────────────────┐
│                                                                 │
│                     DETALHES DO ESTUDANTE         [VOLTAR]     │
│                                                                 │
├────────────┬────────────────────────────────────────────────────┤
│            │                                                     │
│  ████ MENU ████│  ┌─────────────────────────────────────────┐  │
│            │  │  │                                         │  │
│  [ Home ]  │  │  │   ID: 1                                  │  │
│  [ Listar] │  │  │   Nome: Joao Silva                      │  │
│  [ Criar ] │  │  │   UserID: 12345                          │  │
│            │  │  │   Nascimento: 15/05/1990                 │  │
│            │  │  │                                         │  │
│            │  │  │   [═══════════]  [═══════════]          │  │
│            │  │  │   [  EDITAR  ]  [  EXCLUIR  ]           │  │
│            │  │  │    (Preto)       (Vermelho)             │  │
│            │  │  │                                         │  │
│            │  │  └─────────────────────────────────────────┘  │
│            │                                                     │
├────────────┴────────────────────────────────────────────────────┤
│ Status: Visualizando registro ID 1                               │
└─────────────────────────────────────────────────────────────────┘
```

---

## Mockup 5: Popup de Confirmacao (Rebuild Index)

```
┌─────────────────────────────────────────────────────────────────┐
│                                                                 │
│                    ┌────────────────────────────────┐          │
│                    │     ATENCAO                    │          │
│                    │                                │          │
│                    │  O indice precisa ser         │          │
│                    │  reconstruido (10+ registros)│          │
│                    │                                │          │
│                    │  [ SIM ]   [ NAO ]             │          │
│                    │  (Verde)   (Vermelho)           │          │
│                    └────────────────────────────────┘          │
│                                                                 │
└─────────────────────────────────────────────────────────────────┘
```

---

## Resumo de Cores

- **Preto** = Elementos estruturais, botoes secundarios, texto
- **Branco** = Fundos, areas de conteudo  
- **Verde** = Acoes positivas (cadastrar, confirmar, novo)
- **Vermelho** = Acoes negativas (cancelar, excluir, erro)

## Estrutura de Layout

```
┌──────────┬──────────────────────────────────────────┐
│ SIDEBAR  │           AREA PRINCIPAL                  │
│ (150px)  │           (flexivel)                      │
│          │                                          │
│ - Logo   │  - Title bar                             │
│ - Nav    │  - Conteudo da view                      │
│ - Status │  - Feedback/Status                       │
└──────────┴──────────────────────────────────────────┘
```

---

*Mockups criados para o projeto TP AEDS III*
*Design: Minimalista 4-cores (Preto, Branco, Verde, Vermelho)*
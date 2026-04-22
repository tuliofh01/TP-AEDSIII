# Arquitetura GUI - Sistema de Matrícula AEDS III

## Visão Geral

A interface gráfica é construída usando uma combinação de:
- **HelloImGui**: Wrapper C++ para criar janelas e gerenciar o loop de eventos
- **Lua como DSL**: Views definidas em arquivos Lua sem recompilar C++
- **ImGui Bindings**: Funções minimalistas expostas para Lua controlarem a UI

---

## Estrutura de Camadas

```
┌─────────────────────────────────────────┐
│           HelloImGui Runner            │
│  (Gerencia janela, loop, eventos)      │
└────────────────┬────────────────────────┘
                 │
                 ▼
┌─────────────────────────────────────────┐
│         Lua Router (router.lua)         │
│  (Navegação entre views)                │
└────────────────┬────────────────────────┘
                 │
        ┌────────┼────────┬──────────┐
        ▼        ▼        ▼         ▼
    ┌────────┐ ┌──────┐ ┌───────┐ ┌──────────┐
    │ Main   │ │Student│ │Student│ │ Student  │
    │ Menu   │ │Create │ │ List  │ │ Detail   │
    └────────┘ └──────┘ └───────┘ └──────────┘
        │        │        │         │
        └────────┴────────┴─────────┘
                 │
                 ▼
┌─────────────────────────────────────────┐
│      ImGui Bindings (C++ -> Lua)         │
│  (Begin, Text, Button, InputText, etc)  │
└─────────────────────────────────────────┘
```

---

## Fluxo de Renderização

1. **main.cpp**: Configura HelloImGui com callback `RenderUI`
2. **router.lua**: Função `RenderUI()` que decide qual view renderizar
3. **Views**: Cada arquivo Lua renderiza sua interface
4. **Bindings**: Funções C++ de ImGui chamadas pelo Lua

---

## Binding de DataManager

O `DataManager` é exposto para Lua via `luaaa`:

```cpp
// main.cpp
luaaa::LuaClass<DataManager> luaDM(L, "DataManager");
luaDM.ctor();
luaDM.fun("createStudent", &DM::createStudent);
luaDM.fun("readStudent", &DM::readStudent);
luaDM.fun("listAll", &DM::listAll);
// ... etc
```

Uso em Lua:
```lua
local dm = getDataManager()
dm:createStudent("Joao Silva", 1, 15051990)
local students = dm:listAll()
```

---

## Tema 4 Cores

O tema minimalista usa exatamente 4 cores:

| Cor | Hex | Uso |
|-----|-----|-----|
| Preto | #000000 | Texto, bordas, separadores |
| Branco | #FFFFFF | Fundo, áreas de conteúdo |
| Verde | #00FF00 | Ações positivas (cadastrar, confirmar) |
| Vermelho | #FF0000 | Ações negativas (cancelar, excluir, erro) |

Implementado em `common.lua`:
```lua
local theme = {
    Black = {0, 0, 0},
    White = {1, 1, 1},
    Red = {1, 0, 0},
    Green = {0, 1, 0}
}
```

---

## Views Disponíveis

### 1. MainMenu.lua
- Menu inicial com botões de ação
- Exibe contagem de registros ativos

### 2. StudentCreate.lua
- Formulário de cadastro
- Campos: Nome, UserID, Data Nascimento

### 3. StudentList.lua
- Lista todos estudantes ativos
- IDs recalculados dinamicamente
- Ações: Editar, Excluir

### 4. StudentDetail.lua
- Visualiza detalhes de um estudante
- Ações: Editar, Excluir

---

## Arquivos Principais

| Arquivo | Responsabilidade |
|---------|------------------|
| `router.lua` | Navegação e renderização principal |
| `common.lua` | Funções compartilhadas, theme |
| `ImguiBindings.cpp` | bindings C++ de ImGui para Lua |

---

## Como Adicionar Nova View

1. Criar `src/views/NomeView.lua`
2. Exportar função `render()`
3. Adicionar em `router.lua`:
```lua
elseif currentView == "nome" then
    require("src.views.NomeView").render()
end
```

---

*Documento criado para o projeto TP AEDS III - Arquitetura GUI*
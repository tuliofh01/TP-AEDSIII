# Documentação da Interface Gráfica (GUI)

> Sistema de Matrícula Acadêmica - Documentação Técnica da GUI
> Last Updated: 2026-05-24

---

## 1. Visão Geral da Arquitetura GUI

A interface gráfica do sistema é construída utilizando uma combinação de tecnologias que permitem alta flexibilidade e scriptabilidade:

```
+----------------+     +------------------+     +-------------+
|   C++ Core     | --> |   Lua Runtime   | --> |  Dear ImGui |
|  (DataManager)|     |  (DSL/Views)    |     |  (Render)   |
+----------------+     +------------------+     +-------------+
```

### Stack Tecnológico

| Componente | Tecnologia | Propósito |
|------------|-----------|-----------|
| **Framework GUI** | Dear ImGui | Renderização inmediata mode |
| **Wrapper GUI** | hello_imgui | Abstração de janela e loop principal |
| **Linguagem de_script** | Lua 5.x | DSL para definições de UI |
| **Bindings C++/Lua** | ruaaa | Integração C++/Lua |
| **Bindings ImGui** | ImguiBindings.cpp | API ImGui disponível em Lua (minimal, custom) |

---

## 2. Estrutura de Arquivos da GUI

```
src/view/scripts/views/
├── router.lua            # Controlador principal de navegação
├── common.lua            # Sidebar, navegação, helpers
├── MainMenu.lua          # Menu principal com stats
├── StudentCreate.lua     # Cadastro de aluno (nome, email, cpf, data, curso, ano)
├── StudentList.lua       # Listar alunos com exclusão
├── StudentDetail.lua     # Consultar aluno por ID
├── TeacherCreate.lua     # Cadastro de professor (nome, email, cpf, depto, especializacao)
├── TeacherList.lua       # Listar professores com exclusão
├── SubjectCreate.lua     # Cadastro de disciplina (nome, codigo, creditos, professor)
├── SubjectList.lua       # Listar disciplinas com exclusão
└── EnrollmentList.lua    # Gerenciar matrículas (matricular, nota, consultas)
```

### 2.1 router.lua

O `router.lua` é o arquivo central que coordena a navegação entre views:

```lua
-- carregar módulos de view
local MainMenu = require("src.views.MainMenu")
local StudentView = require("src.views.StudentView")

-- estado do roteador
local currentView = "menu"

-- função global chamada por C++ via HelloImGui
function RenderUI()
    if currentView == "menu" then
        MainMenu.render()
    elseif currentView == "student" then
        StudentView.render()
    end
end
```

**Responsabilidades**:
- Carregar módulos de view via `require()`
- Gerenciar estado de navegação (`currentView`)
- Implementar função global `RenderUI()` chamada pelo loop C++
- Passar dados entre views via `appData` table

### 2.2 Estrutura de uma View

Cada view Lua segue o padrão de módulo que retorna uma tabela com métodos:

```lua
-- StudentView.scripts
local M = {}

local state = {
    editingStudent = nil,
    searchId = 0
}

function M.render()
    -- Renderiza UI usando imgui bindings
    if imgui.Begin("Gerenciamento de Estudantes", true) then
        imgui.Text("Gerencie os estudantes do sistema")
        imgui.Separator()
        -- ... campos de input, botões, etc.
        imgui.End()
    end
end

function M.setStudent(student)
    state.editingStudent = student
end

return M
```

**Convenções**:
- Módulo retorna tabela com `M`
- `render()` é a função principal chamada pelo router
- State é mantido local ao módulo (encapsulamento)
- Funções de callback prefixadas com `on` (ex: `onSave()`)

---

## 3. Api de Binding ImGui

### 3.1 Window Management

| Função Lua | Descrição |
|-----------|-----------|
| `imgui.Begin(name, open, flags)` | Cria janela |
| `imgui.End()` | Finaliza janela atual |
| `imgui.SetNextWindowPos(x, y, cond)` | Posição da próxima janela |
| `imgui.SetNextWindowSize(w, h, cond)` | Tamanho da próxima janela |
| `imgui.SetWindowSize(w, h)` | Tamanho da janela atual |

### 3.2 Widgets Básico

| Função Lua | Descrição |
|-----------|-----------|
| `imgui.Text(text)` | Texto simples |
| `imgui.TextColored(r, g, b, a, text)` | Texto colorido |
| `imgui.Separator()` | Linha separadora |
| `imgui.SameLine()` | Widgets na mesma linha |
| `imgui.NewLine()` | Nova linha |
| `imgui.Dummy(w, h)` | Espaçamento |

### 3.3 Input Widgets

| Função Lua | Descrição |
|-----------|-----------|
| `imgui.InputText(label, buffer, size)` | Input de texto |
| `imgui.InputInt(label, value)` | Input de inteiro |
| `imgui.InputFloat(label, value)` | Input de float |
| `imgui.Checkbox(label, checked)` | Checkbox |
| `imgui.Button(label)` | Botão |
| `imgui.SmallButton(label)` | Botão pequeno |

### 3.4 Layout

| Função Lua | Descrição |
|-----------|-----------|
| `imgui.BeginGroup()` | Grupo de widgets |
| `imgui.EndGroup()` | Finaliza grupo |
| `imgui.GetWindowSize()` | Retorna `{w, h}` |
| `imgui.GetContentRegionMax()` | Retorna `{w, h}` da região |

### 3.5 Exemplo Completo

```lua
function M.render()
    imgui.SetNextWindowPos(100, 100, "Always")
    imgui.SetNextWindowSize(400, 300, "Always")
    
    if imgui.Begin("Cadastro de Estudante", true, {"NoResize"}) then
        imgui.Text("Dados Pessoais")
        imgui.Separator()
        
        -- Nome
        imgui.Text("Nome:")
        imgui.SameLine()
        imgui.InputText("##nome", nameBuffer, 50)
        
        -- Data de Nascimento
        imgui.Text("Nascimento (DDMMYYYY):")
        imgui.SameLine()
        imgui.InputInt("##nasc", birthDateBuffer)
        
        imgui.Separator()
        
        -- Botões
        if imgui.Button("Salvar") then
            onSave()
        end
        imgui.SameLine()
        if imgui.Button("Cancelar") then
            onCancel()
        end
        
        imgui.End()
    end
end
```

---

## 4. Integração C++/Lua

### 4.1 Fluxo de Inicialização

```
main.cpp
  ├─ luaL_newstate()          → Cria estado Lua
  ├─ luaL_openlibs()          → Carrega libs padrão
  ├─ LoadImguiBindings(L)    → Bindings ImGui → Lua
  ├─ luaaa::LuaClass<DataManager>
  │   └─ .ctor(), .fun(), .get()
  │       → DataManager exposto ao Lua
  ├─ luaL_dofile(router.lua)  → Carrega router
  └─ HelloImGui::Run()
      └─ RenderUI() callback → Chama RenderUI() Lua
```

### 4.2 Expondo Classe C++ ao Lua

```cpp
// main.cpp
#include "../libs/luaaa/luaaa.hpp"

DataManager myDataManager;

// Classe DataManager exposta como "DataManager" no Lua
luaaa::LuaClass<DataManager> luaDataMgr(L, "DataManager");
luaDataMgr.ctor();
luaDataMgr.fun("processItem", &DataManager::processItem);
luaDataMgr.get("status", &DataManager::getStatus);
luaDataMgr.get("count", &DataManager::getCount);

// Instância única disponível via getDataManager()
luaaa::LuaModule(L).fun("getDataManager", [&]() -> DataManager* {
    return &myDataManager;
});
```

### 4.3 Acessando no Lua

```lua
-- No router.scripts ou outras views
local dm = getDataManager()
dm:processItem("test item")
print(dm.status)  -- "processing: test item"
print(dm.count)  -- 1
```

---

## 5. Padrões de Navegação

### 5.1 Navegação por Callback

```lua
-- router.scripts
local currentView = "menu"

function navigateTo(viewName)
    currentView = viewName
end

function RenderUI()
    if currentView == "menu" then
        -- Menu.render() retorna opção selecionada
        local selected = Menu.render()
        if selected == 1 then
            navigateTo("student")
        elseif selected == 2 then
            navigateTo("course")
        end
    elseif currentView == "student" then
        StudentView.render()
    end
end
```

### 5.2 Passing Data Entre Views

```lua
-- appData table global (definida no router.scripts)
local appData = {}

function setAppData(key, value)
    appData[key] = value
end

function getAppData(key)
    return appData[key]
end

-- Uso:
-- StudentView.render() chama setAppData("selected_student", student)
-- CourseView.render() chama getAppData("selected_student")
```

---

## 6. Criando Novas Views

### 6.1 Estrutura Mínima

```lua
-- src/views/NewView.scripts
local M = {}

local state = {
    -- variáveis de estado locais
}

function M.render()
    -- Implementação da UI
end

-- Funções públicas
function M.setData(data)
    state.data = data
end

function M.getData()
    return state.data
end

return M
```

### 6.2 Registrando no Router

```lua
-- router.scripts
local NewView = require("src.views.NewView")

-- No renderContent():
elseif currentView == "new" then
    NewView.render()
end
```

---

## 7. Componentes UI Comuns

### 7.1 common.lua

O arquivo `common.lua` fornece componentes reutilizáveis:

| Função | Descrição |
|--------|-----------|
| `common.ShowWelcomeMessage(title, msg)` | Mensagem de boas-vindas |
| `common.ShowLoadingSpinner(text)` | Spinner de carregamento |
| `common.ShowErrorDialog(title, msg)` | Dialog de erro |
| `common.ShowConfirmDialog(title, msg, callback)` | Dialog de confirmação |
| `common.CreateStatusBar()` | Barra de status |
| `common.GetTheme()` | Retorna tabela de tema |

### 7.2 Sistema de Tema

```lua
-- common.scripts
local theme = {
    primary = {0.2, 0.4, 0.8, 1.0},    -- RGBA
    secondary = {0.4, 0.4, 0.4, 1.0},
    error = {0.8, 0.2, 0.2, 1.0},
    success = {0.2, 0.8, 0.2, 1.0}
}
```

---

## 8. Pipeline de Renderização

```
+----------------+     +------------------+     +-------------+
|  HelloImGui    | --> |   C++ Callback  | --> |  RenderUI() |
|  Main Loop     |     |  ShowGui       |     |  (Lua)      |
+----------------+     +------------------+     +-------------+
                                                    |
                                                    v
                                          +-----------------+
                                          |   View Router   |
                                          | (router.lua)   |
                                          +-----------------+
                                                    |
                                                    +--------+--------+
                                                    |        |        |
                                                    v        v        v
                                              +--------+ +--------+ +--------+
                                              | Menu   | |Student| | Course |
                                              | View   | | View  | | View  |
                                              +--------+ +--------+ +--------+
```

---

## 9. Troubleshooting

### 9.1 Lua: File Not Found

**Erro**: `cannot open src/views/router.lua`

**Solução**: Verificar caminho em main.cpp e diretório de execução

### 9.2 ImGui: Stack Desbalanceado

**Erro**: Janelas não aparecem ou erros de stack

**Solução**: Garantir que cada `imgui.Begin()` tem `imgui.End()`

### 9.3 DataManager: Incomplete Type

**Erro**: `template argument must be a complete class`

**Solução**: Incluir header completo, não forward declaration

---

## 10. Referências

- [Dear ImGui](https://github.com/ocornut/imgui)
- [hello_imgui](https://github.com/pthom/hello_imgui)
- [Lua 5.x](https://www.lua.org/manual/5.3/)
- [ruaaa](https://github.com/gengyong/luaaa)
- [imgui_lua_bindings](https://github.com/patrickriordan/imgui_lua_bindings)

---

*Este documento é mantido com o projeto. Última atualização: 2026-04-27*
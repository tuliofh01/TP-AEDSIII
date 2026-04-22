# Guia Lua DSL - Sistema de Matrícula AEDS III

## Visão Geral

As interfaces gráficas são escritas em Lua como uma **Domain Specific Language (DSL)**. Isso permite criar e modificar telas sem recompilar o código C++.

---

## Estrutura de Arquivos

```
src/views/
├── router.lua        # Controlador de navegação
├── common.lua        # Funções compartilhadas + tema
├── MainMenu.lua     # Menu inicial
├── StudentCreate.lua # Formulário de cadastro
├── StudentList.lua   # Lista de estudantes
└── StudentDetail.lua # Detalhes/Edição
```

---

## Acesso ao DataManager

O DataManager C++ é exposto para Lua via `getDataManager()`:

```lua
local dm = getDataManager()

-- Operações CRUD
dm:initialize("data/students.dat")
dm:createStudent("Nome", userId, birthDate)
local students = dm:listAll()
local student = dm:readStudent(id)
dm:deleteStudent(id)
local found = dm:searchByName("nome")
```

---

## API do DataManager

| Método | Parâmetros | Retorno | Descrição |
|--------|------------|---------|------------|
| initialize | path | bool | Inicializa arquivos |
| createStudent | name, userId, birthDate | bool | Cria novo estudante |
| readStudent | id | Student? | Lê por ID dinâmico |
| listAll | - | Student[] | Lista todos ativos |
| deleteStudent | id | bool | Soft delete |
| searchByName | name | Student? | Busca por nome |
| getNextDisplayId | - | int | Próximo ID |
| getActiveCount | - | int | Total ativos |
| needsRebuild | - | bool | Precisa reconstruir índice |
| triggerRebuild | - | - | Reconstruir índice |
| ignoreRebuildForSession | - | - | Ignorar rebuild |

---

## Componentes ImGui Disponíveis

Os bindings estão em `imgui.*` (definidos em C++):

### Janelas
```lua
imgui.Begin("Titulo", flags)
imgui.End()
```

### Texto
```lua
imgui.Text("Texto")
imgui.Separator()
imgui.Spacing()
```

### Inputs
```lua
imgui.InputText("Label", "texto inicial")
imgui.InputInt("Label", valor_inicial)
```

### Botões
```lua
if imgui.Button("Label") then
    -- ação
end
imgui.SameLine()
```

### Cores (4 cores theme)
```lua
-- Configurar cores
imgui.PushStyleColor(imgui.Col_WindowBg, {r=1, g=1, b=1, a=1}) -- White
imgui.PushStyleColor(imgui.Col_Text, {r=0, g=0, b=0, a=1}) -- Black
imgui.PushStyleColor(imgui.Col_Button, {r=0, g=1, b=0, a=1}) -- Green
imgui.PopStyleColor()
```

---

## Exemplo: Criar Nova View

### 1. Criar arquivo

```lua
-- src/views/ExemploView.lua
local M = {}

function M.render()
    local dm = getDataManager()
    
    imgui.Text("Minha Nova View")
    imgui.Separator()
    
    if imgui.Button("Voltar") then
        appData.set("currentView", "menu")
    end
    
    imgui.SameLine()
    
    if imgui.Button("Acao") then
        -- fazer algo
    end
end

return M
```

### 2. Registrar no router

Em `router.lua`, adicionar:
```lua
elseif currentView == "exemplo" then
    require("src.views.ExemploView").render()
end
```

---

## Padrões de Código Lua

### Módulo
```lua
local M = {}

function M.funcao()
    -- código
end

return M
```

### Tabela de Dados
```lua
student = {
    id = 1,
    name = "Joao Silva",
    userId = 12345,
    birthDate = 15051990
}
```

### Iteração
```lua
for i, student in ipairs(students) do
    imgui.Text(student.name)
end
```

---

## Estilo 4 Cores

O tema usa exatamente 4 cores:

| Cor | Nome Lua | RGB | Uso |
|-----|----------|-----|-----|
| Preto | Black | (0,0,0) | Texto, bordas |
| Branco | White | (1,1,1) | Fundo |
| Verde | Green | (0,1,0) | Ações positivas |
| Vermelho | Red | (1,0,0) | Ações negativas |

---

## Navegação

O estado da aplicação é gerenciado via `appData`:

```lua
-- Ir para outra view
appData.set("currentView", "studentList")

-- Obter view atual
local current = appData.get("currentView")
```

---

## Validações

Validações são feitas no lado C++:
- Nome não pode ser vazio (retorna false)
- Data deve ser YYYYMMDD válido
- Erro disponível via `dm:getLastError()`

---

## Erros Comuns

```lua
-- Erro: Esquecer de retornar o módulo
-- Correto:
return M

-- Erro: Usar função errada do DM
-- Correto: dm:listAll() não dm.listAll()

-- Erro: Esquecer de chamar End()
-- Correto: imgui.Begin() deve ter imgui.End()
```

---

## Boas Práticas

1. **Sempre verificar**: Use `if imgui.Begin() then ... end`
2. **Separar views**: Cada arquivo = uma view
3. **Usar common.lua**: Funções compartilhadas lá
4. **4 cores only**: Não usar outras cores
5. **Comentários**: Documente funções importantes

---

*Documento criado para o projeto TP AEDS III - Guia Lua DSL*
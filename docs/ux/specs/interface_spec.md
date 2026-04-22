# Especificacao de UX - Sistema de Matricula AEDS III

## Visao Geral

Interface grafica minimalista com **4 cores** para o sistema de matricula academica.

---

## Paleta de Cores (4 Cores)

| Cor | Hex | Usage |
|-----|-----|-------|
| **Preto (Black)** | `#000000` | Texto principal, bordas, separadores |
| **Branco (White)** | `#FFFFFF` | Fundo de janelas, areas de conteudo |
| **Vermelho (Red)** | `#FF0000` | Erros, alertas, botoes de delete/cancel |
| **Verde (Green)** | `#00FF00` | Sucesso, confirmacoes, botoes de acao primaria |

---

## Layout Principal

```
+--------------------------------------------------+
|  [Menu]                          AEDS III        |  <- Title bar (White/Black)
+--------+---------------------------------------+
|        |                                       |
| Menu   |     Area de Conteudo                 |
| Sidebar|                                       |
| (Black)|     (White)                           |
|        |                                       |
| - Home |                                       |
| - List |  [Botoes, Forms, Tabelas]             |
| - Criar|                                       |
|        |                                       |
+--------+---------------------------------------+
|  Status: X registros ativos                    |  <- Status bar
+--------------------------------------------------+
```

---

## Views e Componentes

### 1. MainMenu (Menu Inicial)

```
+------------------------------------------+
| AEDS III - Sistema de Matricula          |
+------------------------------------------+
| [Menu]  |  Bem-vindo ao Sistema de       |
|         |  Matricula Academica            |
| [Home]  |                               |
| [Listar]|  [Cadastrar Estudante] (Green) |
| [Criar] |                               |
|         |  [Listar Estudantes] (Black)   |
+---------+-------------------------------+
| 0 registros ativos                        |
+------------------------------------------+
```

### 2. StudentCreate (Cadastro)

```
+------------------------------------------+
| Cadastrar Estudante          [Voltar]    |
+------------------------------------------+
| [Menu]  |  Nome: [_______________]       |
|         |  UserID: [______]               |
| [Home]  |  Data Nascimento: [______]      |
| [Listar]|                               |
| [Criar] |  [Cadastrar] (Green)           |
|         |  [Cancelar] (Red)              |
+---------+-------------------------------+
+------------------------------------------+
```

### 3. StudentList (Lista)

```
+------------------------------------------+
| Lista de Estudantes           [+ Novo]   |
+------------------------------------------+
| [Menu]  | ID | Nome          | Acoes      |
|         |----+---------------+--------    |
| [Home]  |  1 | Joao Silva    | [E][X]     |
| [Listar]|  2 | Maria Santos  | [E][X]     |
| [Criar] |  3 | Pedro Costa   | [E][X]     |
|         |                               |
+---------+-------------------------------+
| Total: 3 estudantes                       |
+------------------------------------------+
```

### 4. StudentDetail (Detalhes/Edição)

```
+------------------------------------------+
| Detalhes do Estudante         [Voltar]   |
+------------------------------------------+
| [Menu]  |  ID: 1                          |
|         |  Nome: Joao Silva                |
| [Home]  |  UserID: 12345                  |
| [Listar]|  Nascimento: 15/05/1990         |
| [Criar] |                               |
|         |  [Editar] (Black)               |
|         |  [Excluir] (Red)               |
+---------+-------------------------------+
```

---

## Estados Interativos

### Botoes

| Estado | Fundo | Texto | Borda |
|--------|-------|-------|-------|
| Normal | White | Black | Black 1px |
| Hover | Green | Black | Green 2px |
| Active | Black | White | Black 2px |
| Disabled | #CCC | #888 | #CCC |

### Campos de Input

| Estado | Fundo | Texto | Borda |
|--------|-------|-------|-------|
| Normal | White | Black | Black 1px |
| Focus | White | Black | Green 2px |
| Error | White | Red | Red 2px |

---

## Feedback de Acoes

### Sucesso (Verde)
- "Estudante cadastrado com sucesso!"
- Borda verde ao redor da mensagem

### Erro (Vermelho)
- "Nome nao pode ser vazio"
- Borda vermelha ao redor da mensagem

### Loading
- Cursor de espera (hourglass)
- Texto cinza: "Processando..."

---

## Navegacao

- Sidebar: Menu fixo a esquerda (preto/branco)
- Botoes: Green = primary action, Black = secondary, Red = danger
- Separadores: Linhas pretas de 1px

---

## Responsive

- Minima resolucao: 800x600
- Maxima largura do conteudo: 600px
- Sidebar fixa em 150px

---

## Icones (Minimalista)

- `[+]` - Adicionar (Green)
- `[E]` - Editar (Black)  
- `[X]` - Deletar (Red)
- `[<]` - Voltar (Black)
- `[✓]` - Confirmar (Green)
- `[!]` - Alerta (Red)

---

## Documento criado para o projeto TP AEDS III
- Arquitetura: MVC com namespaces flat
- Binding: Lua + ImGui
- Tema: 4 cores minimalista
#!/usr/bin/env lua
-- samples.scripts
-- Script para popular dados de teste no sistema
-- Usage: scripts samples.scripts [count]
-- Default: 20 registros

-- ========================================
-- Dados de teste - Nomes brasileiros
-- ========================================
local nomes = {
    "Joao Silva", "Maria Santos", "Pedro Costa", "Ana Oliveira",
    "Carlos Souza", "Fernanda Lima", "Ricardo Alves", "Juliana Pereira",
    "Marcos Rodrigues", "Beatriz Martins", "Lucas Ferreira", "Gabriela Gomes",
    "Diego Souza", "Camila Rodrigues", "Felipe Costa", "Isabella Silva",
    "Bruno Santos", "Larissa Oliveira", "Gustavo Lima", "Carolina Costa",
    "Andre Martins", "Renata Ferreira", "Thiago Gomes", "Patricia Alves",
    "Vinicius Rodrigues", "Aline Costa", "Rodrigo Silva", "Tatiana Santos",
    "Leandro Oliveira", "Michele Lima"
}

local sobrenomes = {
    "Silva", "Santos", "Oliveira", "Costa", "Lima", "Rodrigues",
    "Ferreira", "Alves", "Gomes", "Martins", "Pereira", "Souza"
}

-- ========================================
-- Gera data aleatoria
-- ========================================
local function geraData()
    local dia = math.random(1, 28)
    local mes = math.random(1, 12)
    local ano = math.random(1990, 2005)
    return ano * 10000 + mes * 100 + dia
end

-- ========================================
-- Gera UserID unico
-- ========================================
local userIds = {}
local function geraUserId()
    local id
    repeat
        id = math.random(10000, 99999)
    until not userIds[id]
    userIds[id] = true
    return id
end

-- ========================================
--main
-- ========================================
local arg = {...}
local count = tonumber(arg[1]) or 20

if count < 1 or count > 30 then
    print("Erro: use samples.scripts [1-30]")
    os.exit(1)
end

print("======================================")
print(" samples.scripts - Dados de Teste")
print("======================================")
print(string.format("Gerando %d registros...", count))

-- Obter DataManager
local ok, dm = pcall(getDataManager)
if not ok or not dm then
    print("Erro: DataManager nao disponivel")
    print("Execute via interface GUI ou carregue primeiro o router.scripts")
    os.exit(1)
end

-- Inicializar
local dataPath = "data/students.dat"
if not dm:initialize(dataPath) then
    print("Erro ao inicializar: " .. dm:getLastError())
    os.exit(1)
end

print(string.format("Arquivo: %s", dataPath))

-- Limpar existentes (opcional - comment se quiser manter)
--[[
local existing = dm:listAll()
for i = #existing, 1, -1 do
    dm:deleteStudent(i)
end
print("Registros anteriores removidos")
--]]

-- Gerar registros
local erros = 0
for i = 1, count do
    local nome = nomes[math.random(#nomes)]
    local userId = geraUserId()
    local data = geraData()
    
    if dm:createStudent(nome, userId, data) then
        local idx = dm:getActiveCount()
        print(string.format("  [%2d] %-25s (userId: %d, nasc: %d)", idx, nome, userId, data))
    else
        erros = erros + 1
        print(string.format("  ERRO ao criar: %s", nome))
    end
end

print("")
print("======================================")
print(string.format("Total gerados: %d", dm:getActiveCount()))
print(string.format("Erros: %d", erros))
print("======================================")

if dm:needsRebuild() then
    print("Nota: Indice precisa de rebuild")
end

print("Dados de teste criados com sucesso!")
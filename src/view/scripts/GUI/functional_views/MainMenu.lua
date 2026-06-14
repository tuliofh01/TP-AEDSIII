-- MainMenu.lua
-- Menu principal do sistema

local M = {}
local common = require("handlers.common")

function M.render()
	local dm = getDataManager()

	imgui.Text("Sistema de Matricula Academica")
	imgui.Separator()
	imgui.Spacing()

	imgui.TextWrapped("Bem-vindo ao sistema de gestao academica.")
	imgui.Spacing()

	imgui.TextWrapped("Selecione uma opcao na barra lateral para comecar.")
	imgui.Spacing()
	imgui.Spacing()

	-- Stats
	imgui.Text("Resumo:")
	imgui.Separator()
	imgui.Spacing()

	if dm then
		local sc = dm:getActiveCount('S')
		local tc = dm:getActiveCount('T')
		local bc = dm:getActiveCount('B')
		common.textColored("Estudantes ativos: " .. tostring(sc), common.COLORS.Green)
		common.textColored("Professores ativos: " .. tostring(tc), common.COLORS.Green)
		common.textColored("Disciplinas ativas: " .. tostring(bc), common.COLORS.Green)
	end

	imgui.Spacing()
	imgui.Spacing()

	-- Quick actions
	imgui.Text("Acoes Rapidas:")
	imgui.Separator()
	imgui.Spacing()

	if common.button("Cadastrar Estudante", common.COLORS.Green, 200, 30) then
		common.goTo("create")
	end
	imgui.Spacing()

	if common.button("Cadastrar Professor", common.COLORS.Green, 200, 30) then
		common.goTo("tcreate")
	end
	imgui.Spacing()

	if common.button("Cadastrar Disciplina", common.COLORS.Green, 200, 30) then
		common.goTo("screate")
	end
	imgui.Spacing()

	if common.button("Matricular Aluno", common.COLORS.Black, 200, 30) then
		common.goTo("elist")
	end
	imgui.Spacing()
end

return M

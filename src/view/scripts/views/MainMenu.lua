-- MainMenu.scripts
-- Menu principal do sistema

local M = {}
local common = require("common")

function M.render()
	imgui.Text("Sistema de Matricula Academica")
	imgui.Separator()
	imgui.Spacing()

	common.textColored("Bem-vindo ao sistema de gestao de estudantes.", common.COLORS.Black)
	imgui.Spacing()

	imgui.Text("Selecione uma opcao na barra lateral para comecar.")
	imgui.Spacing()
	imgui.Spacing()

	-- Acoes rapidas
	imgui.Text("Acoes Rapidas:")
	imgui.Separator()

	if common.button("Cadastrar Estudante", common.COLORS.Green, 200, 35) then
		-- Navegacao via sidebar
	end
	imgui.Spacing()

	if common.button("Listar Estudantes", common.COLORS.Black, 200, 35) then
	end
	imgui.Spacing()

	if common.button("Consultar por Nome", common.COLORS.Black, 200, 35) then
	end
end

return M

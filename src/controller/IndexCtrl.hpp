/**
 * @file IndexCtrl.hpp
 * @brief Controle de indice hash para busca por nome.
 * @namespace project_controller
 */
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <optional>
#include <unordered_map>
#include "../utility/Constants.hpp"
#include "../utility/Enums.hpp"

namespace project_controller {

	/**
	 * @brief Indice hash para busca rapida de estudantes por nome.
	 * 
	 * Usa algoritmo DJB2 para hash.
	 * Reconstrucao a cada multiplo de 10 registros ativos (REBUILD_MODULO).
	 */
	class IndexCtrl {
	public:
		IndexCtrl() = default;
		~IndexCtrl() = default;

		IndexCtrl(const IndexCtrl&) = delete;
		IndexCtrl& operator=(const IndexCtrl&) = delete;
		IndexCtrl(IndexCtrl&&) noexcept = default;
		IndexCtrl& operator=(IndexCtrl&&) noexcept = default;

		/**
		 * @brief Inicializa indice a partir de arquivo .idx.
		 * @param idxPath Caminho do arquivo de indice
		 * @return true se bem-sucedido
		 */
		[[nodiscard]] bool initialize(const std::string& idxPath);

		/**
		 * @brief Salva indice em arquivo binario.
		 * @return true se bem-sucedido
		 */
		[[nodiscard]] bool save() const;

		/**
		 * @brief Carrega indice de arquivo binario.
		 * @return true se bem-sucedido
		 */
		[[nodiscard]] bool load();

		/**
		 * @brief Limpa indice (zera mapa e profundidade).
		 */
		void clear();

		/**
		 * @brief Insere entrada no indice.
		 * @param name Nome do estudante
		 * @param id ID do registro
		 * @param offset Offset no arquivo de dados
		 * @return true se bem-sucedido
		 */
		[[nodiscard]] bool insert(const std::string& name, int32_t id, size_t offset);

		/**
		 * @brief Busca offset por nome normalizado.
		 * @param name Nome a buscar
		 * @return Offset ou nullopt se nao encontrado
		 */
		[[nodiscard]] std::optional<size_t> lookup(const std::string& name) const;

		/**
		 * @brief Remove entrada do indice.
		 * @param name Nome a remover
		 * @return true se encontrado e removido
		 */
		[[nodiscard]] bool remove(const std::string& name);

		/**
		 * @brief Reconstrucao completa do indice.
		 */
		void rebuild();

		/**
		 * @brief Verifica se deve reconstruir (multiplo de 10).
		 * @param activeCount Numero de registros ativos
		 * @return true se deve reconstruir
		 */
		[[nodiscard]] bool shouldRebuild(uint32_t activeCount) const;

		/**
		 * @brief Ignora reconstrucao para sessao atual.
		 */
		void ignoreRebuild();

		/**
		 * @brief Verifica se reconstrucao foi ignorada.
		 * @return true se ignorada
		 */
		[[nodiscard]] bool isRebuildIgnored() const;

	private:
		/**
		 * @brief Hash DJB2 para strings.
		 * @param name String de entrada
		 * @return Hash de 32 bits
		 */
		[[nodiscard]] static uint32_t djb2(const std::string& name);

		/**
		 * @brief Normaliza nome para lowercase (padrao de busca).
		 * @param name Nome original
		 * @return Nome em lowercase
		 */
		[[nodiscard]] static std::string normalize(const std::string& name);

		std::unordered_map<std::string, size_t> map_;
		std::string idxPath_;
		uint32_t depth_ = project_utility::IDX_INITIAL_DEPTH;
		bool rebuildIgnored_ = false;
	};

} // namespace project_controller
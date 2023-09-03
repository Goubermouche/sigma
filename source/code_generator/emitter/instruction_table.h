#pragma once
#include <utility/memory.h>
#include <utility/macros.h>

namespace code_generator {
	using namespace utility::types;

	/**
	 * \brief Base op code class. Contains information about a combination of an operation
	 * and a series of operands. A maximum of 5 operands is supported.
	 * \tparam operation Base type of the operation tag
	 * \tparam operand Base type of the operand tag
	 */
	template<typename operation, typename operand>
	class op_code {
	public:
		using operation_type = operation;
		using operand_type = operand;
		using op_code_type = op_code<operation_type, operand_type>;

		struct hash {
			u64 operator()(const op_code_type& op) const {
				u64 hash_value = 0;

				hash_value ^= std::hash<operation>()(op.m_operation);
				hash_value ^= std::hash<u8>()(op.m_operand_count);

				for (u8 i = 0; i < op.m_operand_count; ++i) {
					hash_value ^= std::hash<operand>()(op.m_operands[i]);
				}

				return hash_value;
			}
		};

		struct equal {
			bool operator()(const op_code_type& left, const op_code_type& right) const {
				if (left.m_operation != right.m_operation) {
					return false;
				}

				if (left.m_operand_count != right.m_operand_count) {
					return false;
				}

				for (u8 i = 0; i < left.m_operand_count; ++i) {
					if (left.m_operands[i] != right.m_operands[i]) {
						return false;
					}
				}

				return true;
			}
		};

		op_code(operation_type operation, std::initializer_list<operand_type> operands)
			: m_operation(operation), m_operand_count(operands.size()) {
			ASSERT(m_operand_count <= 5, "cannot create an op code with more than 5 operands");
			std::copy(operands.begin(), operands.end(), m_operands.begin());
		}

		operation_type get_operation() const {
			return m_operation;
		}

		u8 get_operand_count() const {
			return m_operand_count;
		}

		const std::array<operand_type, 5>& get_operands() const {
			return m_operands;
		}
	private:
		operation_type m_operation;
		u8 m_operand_count;
		std::array<operand_type, 5> m_operands;
	};

	/**
	 * \brief Instruction table class. Contains mappings from operations and operand
	 * combinations to machine byte code.
	 * \tparam operation_base Base type of the operation tag
	 * \tparam operand_base Base type of the operand tag
	 */
	template<typename operation_base, typename operand_base>
	class instruction_table {
	public:
		using operation_type = operation_base;
		using operand_type = operand_base;
		using op_code_type = op_code<operation_type, operand_type>;
		using instruction_list = std::initializer_list<std::pair<op_code_type, std::vector<utility::byte>>>;

		/**
		 * \brief Constructs a new instruction table containing the specified \a instructions.
		 * \param instructions Instruction mappings to insert
		 */
		instruction_table(const instruction_list& instructions)
			: m_instructions(instructions.begin(), instructions.end()) {}

		const std::vector<utility::byte>& operator[](const op_code_type& key) {
			return m_instructions[key];
		}

		template<typename... operand_types>
		const std::vector<utility::byte>& at(operation_type operation, operand_types... operands) const {
			const op_code_type key(operation, { operands... });
			return m_instructions.at(key);
		}
	private:
		std::unordered_map<
			op_code_type,
			std::vector<utility::byte>,
			typename op_code_type::hash,
			typename op_code_type::equal
		> m_instructions;
	};
}

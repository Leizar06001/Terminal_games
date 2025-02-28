
all: morp tc ascars infos

re: fclean all

morp:
	@echo ">>> Building TicTacToe...\n"
	@$(MAKE) --no-print-directory -C TicTacToe
	@cp TicTacToe/morpion ./morpion

tc:
	@echo ">>> Building battleship...\n"
	@$(MAKE) --no-print-directory -C battleship
	@cp battleship/toucher_couler ./toucher_couler

ascars:
	@echo ">>> Building Ascars...\n"
	@$(MAKE) --no-print-directory -C Traffic
	@cp Traffic/ascars ./ascars

infos:
	@echo ">>> Done !\n\n"
	@echo "> To play TicTacToe,      type ./morpion"
	@echo "> To play Toucher-Couler, type ./toucher_couler"
	@echo "> To play Ascars,         type ./ascars"

fclean:
	@echo ">>> Cleaning ALL..."
	@$(MAKE) --no-print-directory -C TicTacToe fclean
	@$(MAKE) --no-print-directory -C battleship fclean
	@$(MAKE) --no-print-directory -C Traffic fclean
	@rm -f morpion toucher_couler ascars
	@echo "🗑️  Clean completed.\n"

clean:
	@echo ">>> Cleaning up..."
	@$(MAKE) --no-print-directory -C TicTacToe clean
	@$(MAKE) --no-print-directory -C battleship clean
	@$(MAKE) --no-print-directory -C Traffic clean

.PHONY: all morp tc ascars clean re infos

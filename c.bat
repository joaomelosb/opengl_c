@echo off

gcc %* -Wall -Wextra -pedantic-errors -lSDL2 -lglew32 -lopengl32 -L.

# TODO

> Mettre a jour les chemins lors du placement de nouvelles routes pour verifier si l'on aurait créé des chemins plus courts
> Feux tricolores
> Mecanique de jeu

# t_path paths[MAX_ORIG][MAX_DEST] >> contient tous les chemins
La seule chose malloc est [paths.steps]
Dans paths on a [*paths->orig] et [*paths->dest] pour retrouver facilement origin et dest
On a un [*next_in_tile] pour chainer les differents paths sur chaque tuile de map

# TOUS LES POINTEURS SUR PATHS POINTENT VERS CE TABLEAU DE T_PATH, pas de malloc

# t_elem origs[MAX_ORIG] >> contient les infos de chaque origines
# t_elem dests[MAX_DEST] >> contient les infos de chaque dests
On a un pointeur vers la liste des paths utilises par ces points [*origs->paths]

# t_map_tile mapt[MAX_H][MAX_W] >> contient les infos de chaque tuile de la map
[.type]     >> Type de la tuile
[.action]   >> 'r' si la tuile est en attente de suppression
[.*paths]    >> La liste des chemins sur cette tuile


struct noeud{
    float cout_g, cout_h, cout_f;
    std::pair<int,int> parent;    // 'adresse' du parent (qui sera toujours dans la map fermée)
};

struct point{
    int x,y;
};

typedef map< pair<int,int>, noeud> l_noeud;

SDL_Surface *s = SDL_LoadBMP("carte.bmp");

SDL_SaveBMP(s, "resultat.bmp");

/* calcule la distance entre les points (x1,y1) et (x2,y2) */
float distance(int x1, int y1, int x2, int y2){
    /* distance euclidienne */
    return sqrt((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2));
 
    /* carré de la distance euclidienne */
    /* return (x1-x2)*(x1-x2) + (y1-y2)*(y1-y2); */
}

bool deja_present_dans_liste(pair<int,int> n, l_noeud& l){
    l_noeud::iterator i = l.find(n);
    if (i==l.end())
        return false;
    else
        return true;
}

void ajouter_cases_adjacentes(pair <int,int>& n){
    noeud tmp;
    /* on met tous les noeuds adjacents dans la liste ouverte (+vérif) */
    for (int i=n.first-1; i<=n.first+1; i++){
        if ((i<0) || (i>=s->w))  /* en dehors de l'image, on oublie */
            continue;
        for (int j=n.second-1; j<=n.second+1; j++){
            if ((j<0) || (j>=s->h))   /* en dehors de l'image, on oublie */
                continue;
            if ((i==n.first) && (j==n.second))  /* case actuelle n, on oublie */
                continue;
 
            if (*((Uint8 *)s->pixels + j * s->pitch + i * s->format->BytesPerPixel) == NOIR)
                /* obstacle, terrain non franchissable, on oublie */
                continue;
 
            pair<int,int> it(i,j);
            if (!deja_present_dans_liste(it, liste_fermee)){
                /* le noeud n'est pas déjà présent dans la liste fermée */
 
                /* calcul du cout G du noeud en cours d'étude : cout du parent + distance jusqu'au parent */
                tmp.cout_g = liste_fermee[n].cout_g + distance(i,j,n.first,n.second);  
 
                /* calcul du cout H du noeud à la destination */
                tmp.cout_h = distance(i,j,arrivee.x,arrivee.y);
                tmp.cout_f = tmp.cout_g + tmp.cout_h;
                tmp.parent = n;
 
                if (deja_present_dans_liste(it, liste_ouverte)){
                    /* le noeud est déjà présent dans la liste ouverte, il faut comparer les couts */
                    if (tmp.cout_f < liste_ouverte[it].cout_f){
                        /* si le nouveau chemin est meilleur, on met à jour */
                        liste_ouverte[it]=tmp;
                    }
 
                    /* le noeud courant a un moins bon chemin, on ne change rien */
 
 
                }else{
                    /* le noeud n'est pas présent dans la liste ouverte, on l'y ajoute */
                    liste_ouverte[pair<int,int>(i,j)]=tmp;
                }
            }
        }
    }
}

pair<int,int> meilleur_noeud(l_noeud& l){
    float m_coutf = l.begin()->second.cout_f;
    pair<int,int> m_noeud = l.begin()->first;
 
    for (l_noeud::iterator i = l.begin(); i!=l.end(); i++)
        if (i->second.cout_f< m_coutf){
            m_coutf = i->second.cout_f;
            m_noeud = i->first;
        }
 
    return m_noeud;
}

void ajouter_liste_fermee(pair<int,int>& p){
    noeud& n = liste_ouverte[p];
    liste_fermee[p]=n;
 
    /* il faut le supprimer de la liste ouverte, ce n'est plus une solution explorable */
    if (liste_ouverte.erase(p)==0)
        cerr << "Erreur, le noeud n'apparaît pas dans la liste ouverte, impossible à supprimer" << endl;
    return;
}

void retrouver_chemin(){
    /* l'arrivée est le dernier élément de la liste fermée */
    noeud& tmp = liste_fermee[std::pair<int, int>(arrivee.x,arrivee.y)];
 
    struct point n;
    pair<int,int> prec;
    n.x = arrivee.x;
    n.y = arrivee.y;
    prec.first  = tmp.parent.first;
    prec.second = tmp.parent.second;
    chemin.push_front(n);
 
    while (prec != pair<int,int>(depart.parent.first,depart.parent.first)){
        n.x = prec.first;
        n.y = prec.second;
        chemin.push_front(n);
 
        tmp = liste_fermee[tmp.parent];
        prec.first  = tmp.parent.first;
        prec.second = tmp.parent.second;
    }
}

    arrivee.x = s->w-1;
    arrivee.y = s->h-1;
 
    depart.parent.first  = 0;
    depart.parent.second = 0;
 
    pair <int,int> courant;
 
    /* déroulement de l'algo A* */
 
    /* initialisation du noeud courant */
    courant.first  = 0;
    courant.second = 0;
 
    /* ajout de courant dans la liste ouverte */
    liste_ouverte[courant]=depart;
    ajouter_liste_fermee(courant);
    ajouter_cases_adjacentes(courant);
 
    /* tant que la destination n'a pas été atteinte et qu'il reste des noeuds à explorer dans la liste ouverte */
    while( !((courant.first == arrivee.x) && (courant.second == arrivee.y))
            &&
           (!liste_ouverte.empty())
         ){
 
        /* on cherche le meilleur noeud de la liste ouverte, on sait qu'elle n'est pas vide donc il existe */
        courant = meilleur_noeud(liste_ouverte);
 
        /* on le passe dans la liste fermée, il ne peut pas déjà y être */
        ajouter_liste_fermee(courant);
 
        /* on recommence la recherche des noeuds adjacents */
        ajouter_cases_adjacentes(courant);
    }
 
    /* si la destination est atteinte, on remonte le chemin */
    if ((courant.first == arrivee.x) && (courant.second == arrivee.y)){
        retrouver_chemin();
 
        ecrire_bmp();
    }else{
        /* pas de solution */
    }

# battleship
A ncurses library implementaion of the game Battleship on Linux
* Install ncurses on Ubuntu/Debian: sudo apt-get install libncurses5-dev
* Install gcc compiler: sudo apt-get install gcc
* Compile source code: gcc -o battleship battleship.c -lcurses
* Run the game: ./battleship map1.conf map2.conf map3.conf

# Cerinta 1:
	Am construit un meniu cu 4 butoane (New Game, Continue, Scores, Quit) care se apeleaza din main ca functia mainMenu(...). Acest meniu functioneaza cu ajutorul functiei waitForInput() care a fost implmentata astfel incat sa returneze un cod pentru fiecare tasta apasata (ex: Sageata sus returneaza 1, Q returneaza 6). In functie de ce apasa utilizatorul de la tastatura, meniul se redeseneaza corespunzator. La apasarea tastei enter functia mainMenu() apeleaza alta functie corespunzatoare butonului care era selectat.

# Cerinta 2:
	 Pentru generarea hatrii computer-ului am implementat functia generateBoard() care genereaza random un punct pe harta si o directie si verifica cu functia scanField() daca o nava de lungimea specificata poate fii amplasata acolo. In cazul in care se poate amplasa, functia o amplaseaza cu functia placeShip() si incearca sa plaseze urmatoarele nave, altfel cauta un alt loc pentru nava curenta.
	Pentru harta player-ului se am definit formatul urmator:
		- "." repreznita spatiu liber
		- "x" reprezinta nava
	Un exemplu de configuratie valida se poate vedea in "map1.conf"

# Cerinta 3:
	Pentru aceasta cerinta am desenat UI-ul astfel incat player-ul sa vada ce casuta a calculatorului ataca. Folosind aceeasi functie waitForInput() ca la meniul principal redesenez intreagul ecran dupa apasarea unui buton. 
	Variabila playerTurn retine daca este randul jucatorului de a alege sau nu. Dupa punerea cursorului pe casuta dorita, player-ul apasa ENTER, iar casuta selectata este descoperita. In matricea playerDiscovered se tine cont ce casute din tabla player-ului sunt vizibile pentru calculator, iar in computerDiscovered ce casute poate vedea player-ul. Se tine cont de lovituri atunci cand se schimba tura (ex: daca calculatorul sau player-ul loveste o nava primeste o tura in plus.)
	Mutarea calculatroului se face in functie de dificultatea selectata fie apeland functia attack(), fie functia smartAttack(). Selectarea unei casute de catre computer se face in mod aleator daca este apelata functia attack(). Daca mutarea calculatorului se face prin functia smartAttack(), calculatorul tine cont de ultima lui mutare dupa ce nimereste o nava si incearca sa loveasca in jurul acelui punct. De asemenea, am implementat si o perioada de asteptare dupa mutarea calcultorului de o secunda (3 secunde mi se parea cam mult, iar jocul devenea plictisitor).

# Cerinta 4:
	Randomize map - se face prin apasarea tastei R si dupa fiecare apasare, navele care nu au fost distruse complet se repozitioneaza pe harta. Acest lucru se realizeaza avand un vector atata pentru jucator cat si pentru computer care retine cate nave si de ce tip nu au fost inca distruse complet. Se sterge toata harta in afara de navele distruse, iar apoi incepe procesul de pozitionare al navelor care nu au fost distruse, retinute in vectorul pShipsUp/cShipsUp.
    Functia corespunzatoare este: randomizeMap()
	Destroy in Advance - se face prin apasarea tastei D si apeleaza de 10 ori perechi de atacuri cu functia attack(), verificand de fiecare data daca cineva a castigat. Atacurile sunt generate random si nu tin cont de dificultatea selectata.
    Functia corespunzatoare este: destroyInAdvance()

# Cerinta 5:
	La terminarea jocului se apeleaza functia endScreen() care arata pe ecran castigatorul si cate puncte a strans jucatorul in acel meci. 
	Scorul jucatroului este calculat astfel: 450 * (numar de casute nava descoperite) - 100 * (numar de casute goale descoperite).
	Numarul de nave doborate nu este afisat pe acest ecran deoarece acesta este afisat live in timpul jocului pentru ambele parti, iar dupa terminrea jocului exista un timp inainte de intrarea in End Screen in care jucatorul poate vedea numarul de nave distruse atat de el cat si de oponent.
	Scorul este retinut in HIGHSCORES impreuna cu NICKNAME-ul selectat de jucator dupa apasarea tastei random, daca acesta a doborat un highscore. Highscore-ul se pastreaza si dupa inchiderea aplicatiei in fisierul "scores". Din meniul principal se pot verifica highscor-urile selectand butonul "Scores"

# Bonus-uri:
	- Pentru infrumusetarea aspectului am desenat logo-ul BATTLESHIP pe ecranul principal si logo-ul HIGHSCORES pe ecranul de scoruri. De asemenea am scris controalele fiecarei etape a jocului in partea din stanga jos a ecranului. Am folosit mai multe cuori si simboluri pentru reprezentarea celulelor doborate, goale, ascunse si lovite si am scris in timpul jocului pe ecran a cui tura este si cate nave are doborate fiecare jucator.
	- Am memorat un score list care poate fi accesat din meniul principal si nu se pierde odata cu inchiderea jocului deoarece este salvat in fisierul "scores". (functia highscores())
	- Am implementat un meniu de alegere a hartii la inceputul fiecarui joc care tine cont daca fisierele date in argumente nu exista. (functia chooseMap())
	- Am implementat o functie de schimbare a dificultatii in interiorul meniului de alegere a hartii. Dificultatea usoara face computer-ul sa aleaga casute la intamplare, in timp ce dificultatea grea tine cont de loviturile anterioare, iar atunci cand nimereste o nava cauta in jurul ei (functia smartAttack()).
	- Am implementat in End Screen o metoda de a introduce un Nickname de 3 carcatere (RETRO GAMES STYLE) folosind sagetile si o functie care calculeaza scorul jucatorului in functie de cat de bine a jucat in timpul meciului.

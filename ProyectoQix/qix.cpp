#include "raylib.h"
#include <cstdio>//sprintf
#include <cmath>//round

constexpr int TAMANIO_TILE = 16;

/*
TODO: Enemigo bordes
*/

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------

// Definimos el operador de igualdad entre 2 Vector2, para usarlo con la funcion ObtenerTile
static bool operator==(const Vector2& a, const Vector2& b) {
    return (a.x == b.x) && (a.y == b.y);
}
// Definimos el operador de igualdad entre 2 Vector2, para usarlo con la funcion ObtenerTile
static bool operator!=(const Vector2& a, const Vector2& b){
    return (a.x != b.x) || (a.y != b.y);
}

class Tilemap {
public:    
    static constexpr int ANCHO = 20;
    static constexpr int ALTO_MAPA_JUGABLE = 20;
    static constexpr int ALTO = 20;
    // Atajos para hacer mas legible/claro al momento de llamar funciones de ObtenerTile, SetTile
    static constexpr Vector2 TILE_BORDE = {13.0f,16.0f};
    static constexpr Vector2 TILE_RELLENO = {15.0f,6.0f};
    static constexpr Vector2 TILE_VACIO = {0.0f,0.0f};
    static constexpr Vector2 TILE_LINEA_INCOMPLETA = {1.0f,10.0f};
    
    // Metodos


    inline Tilemap(){
        // Copiamos el mapa original en una variable para que al usar el GetArea (el cual implementa el flood-fill) no modifiquemos el mapa que ve el jugador
        for (int i = 0; i < ANCHO * ALTO_MAPA_JUGABLE; i++)
            copiaMapa[i] = mapa[i];
        // Guardamos la cantidad de tiles vacios del mapa al inicio del juego
        for (const auto& tile : mapa)
            if (tile == 1)
                cantidadTilesVaciosInicio++;
    }
    static void SetTile(const int&, const int&, const int&,Tilemap*,const bool&);
    static Vector2 ObtenerTile(const int&,const int&,const Tilemap&,const bool&);
    void Fill(const Vector2&,const int& ,const Vector2&,Tilemap&);
    void GetArea(Tilemap& copiaTilemap, const int& nuevoTile, const Vector2& tileXcambiar, const int& posX, const int& posY, int& contador);
    void EndureceBordes();
    
    inline void CargaTextura(const char* ruta){
        tile_sheet = LoadTexture(ruta);
    }
    // Funcion para reiniciar la copia y evitar errores al usar CalcularArea() varias veces seguidas.
    // Se debe usar siempre despues de usar el CalcularArea()
    inline void ReiniciaCopiaMapa(){
        for (int i = 0; i < ANCHO * ALTO_MAPA_JUGABLE; i++)
            copiaMapa[i] = mapa[i];
    }
    // Dibujar el tilemap en consola
    void Dibujar()const {
        // Dibuja el mapa
        for (int i = 0; i < sizeof(mapa) / sizeof(mapa[0]); i++)
        {
            int posX = i % ANCHO;
            int posY = i / ALTO;
            const Rectangle rectangle = { tiles[mapa[i]].x * TAMANIO_TILE, tiles[mapa[i]].y * TAMANIO_TILE, TAMANIO_TILE, TAMANIO_TILE };
            DrawTextureRec(tile_sheet, rectangle, { static_cast<float>(posX) * TAMANIO_TILE,static_cast<float>(posY) * TAMANIO_TILE }, LIGHTGRAY);
        }

        char buffer[20]{};

        // Dibuja el numero de tiles necesarios para ganar(40% de los vacios iniciales)
        sprintf(buffer, "Debes llenar %d Tiles para GANAR", static_cast<int>(cantidadTilesVaciosInicio * 0.6f));
        DrawText(buffer, 400, 20, 22, BLUE);

        sprintf(buffer,"%hu / %d llenados para ganar",cantidadTilesRellenados,static_cast<int>(cantidadTilesVaciosInicio * 0.6f));
        DrawText(buffer, 400,70,22,WHITE);

        // Dibuja el numero de tiles vacios INICIALES
        sprintf(buffer, "Tiles VACIOS iniciales: %d", cantidadTilesVaciosInicio);
        DrawText(buffer, 400, 50, 22, RED);
        
        // Dibuja el numero de tiles RELLENADOS
        sprintf(buffer, "No. Tiles rellenados: %hu", cantidadTilesRellenados);
        DrawText(buffer, 400, 100, 22, GREEN);
    }

    /// @brief Revisa si la cantidad de tiles vacios en el mapa es menor a 40% y termina el juego
    /// @param gameOver La variable que determina el fin del bucle en el main
    /// @param victoria La variable para mostrar el texto de victoria o derrota
    void RevisaMapa(bool& gameOver,bool& victoria) {
        unsigned short contadorTilesVaciosActuales = 0;
        // Para cada tile, si es vacio o borde no-fijo (ignorando los bordes originales), lo contamos
        for (int i = ANCHO; i < ANCHO * ALTO_MAPA_JUGABLE - ANCHO - 1; i++) {
            if((mapa[i] == 3 || mapa[i] == 1) && i % ANCHO != 0)
            contadorTilesVaciosActuales++;
        }
        cantidadTilesRellenados = cantidadTilesVaciosInicio - contadorTilesVaciosActuales;

        // El jugador debe ganar si el porcentaje de tiles vacíos es menor al 40% de los tiles vacíos iniciales.
        if (contadorTilesVaciosActuales <= std::round(cantidadTilesVaciosInicio * 0.4f)){ // Redondeamos
            gameOver = true;
            victoria = true;
        }
    }
    //Borramos la textura en el destructor
    ~Tilemap() {UnloadTexture(tile_sheet); }

    private:
    static Vector2 tiles[];
    static int mapa[ANCHO * ALTO_MAPA_JUGABLE]; // Actualiza segun el ANCHO * ALTO_MAPA_JUGABLE
    int copiaMapa[ANCHO * ALTO_MAPA_JUGABLE]; // Actualiza segun el ANCHO * ALTO_MAPA_JUGABLE
    Texture2D tile_sheet;
    unsigned short cantidadTilesVaciosInicio = 0;
    unsigned short cantidadTilesRellenados = 0;
};

Vector2 Tilemap::tiles[] = {/*La imagen de los assets tiene 49 tiles de largo y 22 de alto*/
    {13.0f,16.0f}, {0.0f,0.0f},   /* 0 es el borde, 1 es el vacio*/
    {15.0f,6.0f},//2 es el relleno 
    {1.0f,10.0f} //3 es el borde sin fijar (el del modo de dibujado)
};

int Tilemap::mapa[] = {   /// Puede aumentarse
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
    0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
    0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
    0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
    0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
    0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
    0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
    0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
    0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
    0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
    0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
    0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
    0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
    0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
    0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
    0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
    0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
    0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
    0,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,
    0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
};
/// @brief Funcion para obtener el tile dado por las coordenadas del MAPA (inicia en cero)
/// @param x Cordenada x (inicia en cero)
/// @param y Cordenada y (inicia en cero)
/// @param map El mapa del cual se comprobara
/// @param compruebaCopia Retornamos el tile del mapa o de la copia del mapa?
/// @return El tile de la posicion dada en formato de Vector2
Vector2 Tilemap::ObtenerTile(const int& x, const int& y,const Tilemap& map,const bool& compruebaCopia) {
    // Convertimos coordenadas humanas a coordenadas de indices de array

    // Asegurarse de que las coordenadas convertidas estan dentro del rango permitido
    if (x < 0 || x >= ANCHO || y < 0 || y >= ALTO)
        return { 0.0f, 0.0f };

    // Calcular la posicion en el array mapa
    int index = y * ANCHO + x;

    // Verificar que el indice esta dentro del rango del array mapa
    if (index < 0 || index >= sizeof(map.mapa) / sizeof(map.mapa[0]))
        return { 0.0f, 0.0f };

    // Obtener el valor del array mapa
    int tileIndex;
    if(compruebaCopia)
    tileIndex = map.copiaMapa[index];
    else
    tileIndex = map.mapa[index];

    // Verificar que el indice esta dentro del rango del array tiles
    if (tileIndex < 0 || tileIndex >= sizeof(tiles) / sizeof(tiles[0]))
        return { 0.0f, 0.0f };

    
    return tiles[tileIndex];
}

/// @brief Cambia el tile indicado
/// @param tileNuevo El tipo de tile nuevo (Mira Tilemap::tiles)
/// @param x Posicion en x (de cero en adelante)
/// @param y Posicion en y (de cero en adelante)
/// @param map El mapa al cual se le aplicaran los cambios
/// @param cambiaCopia Cambiaremos la copia del mapa o el mapa original?
void Tilemap::SetTile(const int& tileNuevo, const int& x, const int& y,Tilemap* map,const bool& cambiaCopia)
{
    const int indice = y * ANCHO + x;
    if (cambiaCopia)
        map->copiaMapa[indice] = tileNuevo;
    else
        map->mapa[indice] = tileNuevo;
}

/// @brief Flood-Fill
/// @param tilePorCambiar El tile considerado como vacio, es el negro comunmente
/// @param nuevoTile El tile que rellenara los espacios vacios
/// @param pos Coordenada de inicio del Fill
/// @param mapa El mapa al que se le aplicaran los cambios
void Tilemap::Fill(const Vector2& tilePorCambiar,const int& nuevoTile,const Vector2& pos,Tilemap& mapa){
    if (pos.x < 0 || pos.y < 0 || pos.x > ANCHO - 1 || pos.y > ALTO_MAPA_JUGABLE - 1)
        return;
    if (ObtenerTile(pos.x, pos.y, mapa, false) == tilePorCambiar)
    {
        SetTile(nuevoTile, pos.x, pos.y, &mapa,false);
        Fill(tilePorCambiar, nuevoTile, {pos.x, pos.y - 1}, mapa);
        Fill(tilePorCambiar, nuevoTile, {pos.x + 1, pos.y}, mapa);
        Fill(tilePorCambiar, nuevoTile, {pos.x, pos.y + 1}, mapa);
        Fill(tilePorCambiar, nuevoTile, {pos.x - 1, pos.y}, mapa);
    }
}


/// @brief Calcula el area al completar un borde
/// @param copiaTilemap  Tilemap copia
/// @param nuevoTile El tile que sustituira al xCambiar
/// @param tileXcambiar El tile que cambiaremos
/// @param posX Posicion en x a cambiar
/// @param posY Posicion en y a cambiar
/// @param contador El contador donde guardaremos el area
void Tilemap::GetArea(Tilemap& copiaTilemap, const int& nuevoTile, const Vector2& tileXcambiar, const int& posX, const int& posY, int& contador) {
    //(const int& tileNuevo, const int& x, const int& y, Tilemap * map, const bool& cambiaCopia)
    if (posX >= 0 && posX <= Tilemap::ANCHO - 1 && posY >= 0 && posY <= Tilemap::ALTO_MAPA_JUGABLE - 1)
    {
        if (Tilemap::ObtenerTile(posX, posY, copiaTilemap, true) == tileXcambiar)
        {
            SetTile(nuevoTile, posX, posY, &copiaTilemap, true);
            contador++;
            GetArea(copiaTilemap, nuevoTile, tileXcambiar, posX, posY - 1, contador);
            GetArea(copiaTilemap, nuevoTile, tileXcambiar, posX, posY + 1, contador);
            GetArea(copiaTilemap, nuevoTile, tileXcambiar, posX - 1, posY, contador);
            GetArea(copiaTilemap, nuevoTile, tileXcambiar, posX + 1, posY, contador);
        }
    }
}

void Tilemap::EndureceBordes()
{
    // Cambiamos los tiles de camino temporales por tiles de borde
    for(int& tile: mapa){
        if(tile == 3)
        tile = 0;
    }
}

//----------------------------------------------------------------------------------------------------------------------------------

class Personaje {
private:
// Direcciones que nos ayudaran al momento de usar Flood-Fill
enum Direcciones{
    ARRIBA,ABAJO,IZQUIERDA,DERECHA,COMODIN
};
bool dibujando = false;
bool debeHacerFloodFill = false;
Vector2 posInicioDibujando;
Direcciones ultimaDireccion = COMODIN; //TODO: Tal vez el nombre deberia ser "direccionDeMovimiento", seria mas correcto
const unsigned short FRAMES_PARA_UPDATE_DIBUJANDO = 8;
const unsigned short FRAMES_PARA_UPDATE = 15;

protected:
    Vector2 posicion;
    Texture2D tilesheet;

public:
//Constructor que inicia al jugador en (0,0) o en el (x,y) especificad
    Personaje();
    Personaje(const int& xInicial,const int& yInicial) {
        this->posicion.x = xInicial;
        this->posicion.y = yInicial;
    }
    virtual void Update(Tilemap&,const bool&);
    virtual void Mover(Tilemap&);
    virtual void Dibujar() const;
    inline Vector2 GetPosition() const { return posicion; };
    inline void CargaTextura(const char* ruta) {
        tilesheet = LoadTexture(ruta);
    }
    ~Personaje() { UnloadTexture(tilesheet); }
};

Personaje::Personaje() {
    posicion.x = 0.0f;
    posicion.y = 0.0f;
}

void Personaje::Mover(Tilemap &tilemap)
{
    static unsigned short framesCounter = 0;
    framesCounter++;
    // Movimiento libre en el modo de "dibujado"
    if (dibujando)
    {
        if (framesCounter >= FRAMES_PARA_UPDATE_DIBUJANDO) // Si pasaron n frames, podemos movernos. Esto para controlar la velocidad de movimiento y no movernos ultra rapido
        {
            /*
            Si el jugador presiona una direccion, esta en una coordenada valida dentro del mapa,
            el tile en la casilla a la que queremos movernos no es un relleno ni una linea semi-dibujada,
            el jugador no esta intentando regresar por la linea que esta dibujando,
            y si el tile la posicion del jugador es un tile vacio
            ENTONCES cambiamos el tile por un tile de linea semi-dibujada.

            Adicionalmente cambiamos la posicion y la direccion anterior, esto ultimo para evitar
            que durante el modo dibujado el jugador pueda regresar por la linea que esta dibujando
            */

            if (IsKeyDown(KEY_D) && posicion.x < Tilemap::ANCHO - 1 && Tilemap::ObtenerTile(posicion.x + 1, posicion.y, tilemap, false) != Tilemap::TILE_RELLENO && Tilemap::ObtenerTile(posicion.x + 1, posicion.y, tilemap, false) != Tilemap::TILE_LINEA_INCOMPLETA && ultimaDireccion != IZQUIERDA)
            {
                if (Tilemap::ObtenerTile(posicion.x, posicion.y, tilemap, false) == Tilemap::TILE_VACIO)
                {
                    Tilemap::SetTile(3, posicion.x, posicion.y, &tilemap, false);
                    debeHacerFloodFill = true;
                }
                posicion.x++;
                ultimaDireccion = DERECHA;
            }
            else if (IsKeyDown(KEY_A) && posicion.x > 0 && Tilemap::ObtenerTile(posicion.x - 1, posicion.y, tilemap, false) != Tilemap::TILE_RELLENO && Tilemap::ObtenerTile(posicion.x - 1, posicion.y, tilemap, false) != Tilemap::TILE_LINEA_INCOMPLETA && ultimaDireccion != DERECHA)
            {
                if (Tilemap::ObtenerTile(posicion.x, posicion.y, tilemap, false) == Tilemap::TILE_VACIO)
                {
                    Tilemap::SetTile(3, posicion.x, posicion.y, &tilemap, false);
                    debeHacerFloodFill = true;
                }
                posicion.x--;
                ultimaDireccion = IZQUIERDA;
            }
            // Movimiento vertical
            else if (IsKeyDown(KEY_S) && posicion.y < Tilemap::ALTO_MAPA_JUGABLE - 1 && Tilemap::ObtenerTile(posicion.x, posicion.y + 1, tilemap, false) != Tilemap::TILE_RELLENO && Tilemap::ObtenerTile(posicion.x, posicion.y + 1, tilemap, false) != Tilemap::TILE_LINEA_INCOMPLETA && ultimaDireccion != ARRIBA)
            {
                if (Tilemap::ObtenerTile(posicion.x, posicion.y, tilemap, false) == Tilemap::TILE_VACIO)
                {
                    Tilemap::SetTile(3, posicion.x, posicion.y, &tilemap, false);
                    debeHacerFloodFill = true;
                }
                posicion.y++;
                ultimaDireccion = ABAJO;
            }
            else if (IsKeyDown(KEY_W) && posicion.y > 0 && Tilemap::ObtenerTile(posicion.x, posicion.y - 1, tilemap, false) != Tilemap::TILE_RELLENO && Tilemap::ObtenerTile(posicion.x, posicion.y - 1, tilemap, false) != Tilemap::TILE_LINEA_INCOMPLETA && ultimaDireccion != ABAJO)
            {
                if (Tilemap::ObtenerTile(posicion.x, posicion.y, tilemap, false) == Tilemap::TILE_VACIO)
                {
                    Tilemap::SetTile(3, posicion.x, posicion.y, &tilemap, false);
                    debeHacerFloodFill = true;
                }
                posicion.y--;
                ultimaDireccion = ARRIBA;
            }
            framesCounter = 0;
        }
    }
    // Movimiento solo en bordes
    else
    {
        if (framesCounter >= FRAMES_PARA_UPDATE)
        {
            // Movimiento horizontal
            // Si el jugador esta en una coordenada valida dentro del mapa y esta sobre un tile Borde, podemos movernoss
            if (IsKeyDown(KEY_D) && posicion.x < Tilemap::ANCHO - 1)
            {
                // Si el espacio de la derecha es de tipo BORDE podemos desplazarnos hacia el
                if (Tilemap::ObtenerTile(posicion.x + 1, posicion.y, tilemap, false) == Tilemap::TILE_BORDE)
                    posicion.x++;
            }
            else if (IsKeyDown(KEY_A) && posicion.x > 0)
            {
                // Si el espacio de la izquierda es de tipo BORDE podemos desplazarnos hacia el
                if (Tilemap::ObtenerTile(posicion.x - 1, posicion.y, tilemap, false) == Tilemap::TILE_BORDE)
                    posicion.x--;
            }
            // Movimiento vertical
            else if (IsKeyDown(KEY_S) && posicion.y < Tilemap::ALTO_MAPA_JUGABLE - 1)
            {
                // Si el espacio de abajo es de tipo BORDE podemos desplazarnos hacia el
                if (Tilemap::ObtenerTile(posicion.x, posicion.y + 1, tilemap, false) == Tilemap::TILE_BORDE)
                    posicion.y++;
            }
            else if (IsKeyDown(KEY_W) && posicion.y > 0)
            {
                // Si el espacio de la derecha es de tipo BORDE podemos desplazarnos hacia el
                if (Tilemap::ObtenerTile(posicion.x, posicion.y - 1, tilemap, false) == Tilemap::TILE_BORDE)
                    posicion.y--;
            }
            framesCounter = 0;
        }
    }
}

void Personaje::Update(Tilemap &tilemap,const bool& gameOver)
{
    // En caso de que ya hayamos llenado los tiles o los enemigos nos mataron, no actualizamos
    if (gameOver)
        return;

    // Activar el dibujado
    if(IsKeyPressed(KEY_SPACE) && !dibujando){
        dibujando = true;
        posInicioDibujando = posicion;
    }

    Mover(tilemap);

    // Cuando dejamos de dibujar, cambiamos el tile borde temporal por borde solido
    if (dibujando && Tilemap::ObtenerTile(posicion.x, posicion.y, tilemap, false) == Tilemap::TILE_BORDE && debeHacerFloodFill)
    {
        dibujando = false;

        int contador1 = 0, contador2 = 0;
        Tilemap copiaMapa;
        switch (ultimaDireccion)
        {
        case IZQUIERDA: // En caso de que llegamos a un borde siguiendo la direccion <-
            // Calculamos las areas a partir de las posiciones arriba-derecha y abajo-derecha
            copiaMapa.GetArea(copiaMapa, 2, Tilemap::TILE_VACIO, posicion.x + 1, posicion.y - 1, contador1);
            copiaMapa.ReiniciaCopiaMapa();
            copiaMapa.GetArea(copiaMapa, 2, Tilemap::TILE_VACIO, posicion.x + 1, posicion.y + 1, contador2);
            // Flood-fill en base al area menor
            if(contador1 <= 0){
                tilemap.Fill(Tilemap::TILE_LINEA_INCOMPLETA, 1, {posicion.x + 1, posicion.y - 1}, tilemap);
                
            }
            else if(contador2 <= 0){
                tilemap.Fill(Tilemap::TILE_LINEA_INCOMPLETA, 1, {posicion.x + 1, posicion.y + 1}, tilemap);
                
            }
            else if (contador1 < contador2)
                tilemap.Fill(Tilemap::TILE_VACIO, 2, {posicion.x + 1, posicion.y - 1}, tilemap);
            else
                tilemap.Fill(Tilemap::TILE_VACIO, 2, {posicion.x + 1, posicion.y + 1}, tilemap);
            break;
        case DERECHA: // En caso de que llegamos a un borde siguiendo la direccion ->
            // Calculamos las areas a partir de las posiciones arriba-izquierda y abajo-izquierda
            copiaMapa.GetArea(copiaMapa, 2, Tilemap::TILE_VACIO, posicion.x - 1, posicion.y - 1, contador1);
            copiaMapa.ReiniciaCopiaMapa();
            copiaMapa.GetArea(copiaMapa, 2, Tilemap::TILE_VACIO, posicion.x - 1, posicion.y + 1, contador2);
            // Flood-fill en base al area menor
             if(contador1 <= 0 ){
                tilemap.Fill(Tilemap::TILE_LINEA_INCOMPLETA, 1, {posicion.x - 1, posicion.y - 1}, tilemap);
                
            }
            else if(contador2 <= 0){
                tilemap.Fill(Tilemap::TILE_LINEA_INCOMPLETA, 1, {posicion.x - 1, posicion.y + 1}, tilemap);
                
            }
            else if (contador1 < contador2)
                tilemap.Fill(Tilemap::TILE_VACIO, 2, {posicion.x - 1, posicion.y - 1}, tilemap);
            else 
                tilemap.Fill(Tilemap::TILE_VACIO, 2, {posicion.x - 1, posicion.y + 1}, tilemap);
            break;
        case ARRIBA: // En caso de que llegamos a un borde siguiendo la direccion hacia arriba
            // Calculamos las areas a partir de las posiciones abajo-izquierda y abajo-derecha
            copiaMapa.GetArea(copiaMapa, 2, Tilemap::TILE_VACIO, posicion.x - 1, posicion.y + 1, contador1);
            copiaMapa.ReiniciaCopiaMapa();
            copiaMapa.GetArea(copiaMapa, 2, Tilemap::TILE_VACIO, posicion.x + 1, posicion.y + 1, contador2);
            // Flood-fill en base al area menor
             if(contador1 <= 0 ){
                tilemap.Fill(Tilemap::TILE_LINEA_INCOMPLETA, 1, {posicion.x -1, posicion.y + 1}, tilemap);
                
            }
            else if(contador2 <= 0){
                tilemap.Fill(Tilemap::TILE_LINEA_INCOMPLETA, 1, {posicion.x + 1, posicion.y + 1}, tilemap);
                
            }
            else if (contador1 < contador2)
                tilemap.Fill(Tilemap::TILE_VACIO, 2, {posicion.x - 1, posicion.y + 1}, tilemap);
            else 
                tilemap.Fill(Tilemap::TILE_VACIO, 2, {posicion.x + 1, posicion.y + 1}, tilemap);
            break;
        case ABAJO: // En caso de que llegamos a un borde siguiendo la direccion hacia abajo
            // Calculamos las areas a partir de las posiciones arriba-izquierda y arriba-derecha
            copiaMapa.GetArea(copiaMapa, 2, Tilemap::TILE_VACIO, posicion.x - 1, posicion.y - 1, contador1);
            copiaMapa.ReiniciaCopiaMapa();
            copiaMapa.GetArea(copiaMapa, 2, Tilemap::TILE_VACIO, posicion.x + 1, posicion.y - 1, contador2);
            // Flood-fill en base al area menor
             if(contador1 <= 0 ){
                tilemap.Fill(Tilemap::TILE_LINEA_INCOMPLETA, 1, {posicion.x - 1, posicion.y - 1}, tilemap);
                
            }
            else if(contador2 <= 0){
                tilemap.Fill(Tilemap::TILE_LINEA_INCOMPLETA, 1, {posicion.x + 1, posicion.y - 1}, tilemap);
                
            }
            else if (contador1 < contador2)
                tilemap.Fill(Tilemap::TILE_VACIO, 2, {posicion.x - 1, posicion.y - 1}, tilemap);
            else
                tilemap.Fill(Tilemap::TILE_VACIO, 2, {posicion.x + 1, posicion.y - 1}, tilemap);
            break;
        }
        tilemap.EndureceBordes();
        debeHacerFloodFill = false;
        // Reseteamos la ultima direccion para evitar problemas cuando volvamos a activar el modo de dibujado
        ultimaDireccion = COMODIN;
    }
}

void Personaje::Dibujar() const
{
	//TODO: Debug Dibuja coordenadas por ayuda
	char buffer[50];
	sprintf(buffer, "x= %.0f, y=%.0f", posicion.x, posicion.y);
	DrawText(buffer, GetScreenWidth() / 2, GetScreenHeight() * 2 / 5, 22, GREEN);

	Rectangle rectSpritePers = { 27.0f * TAMANIO_TILE, 7.0f * TAMANIO_TILE, TAMANIO_TILE, TAMANIO_TILE };
	DrawTextureRec(tilesheet, rectSpritePers, { posicion.x * TAMANIO_TILE, posicion.y * TAMANIO_TILE }, WHITE);
}
// ----------------------------------------------------------------------------
class Enemigo : public Personaje{
protected:
const unsigned short FRAMES_PARA_UPDATE = 6;
public:
    
    Enemigo(const int &, const int &);
    void Update(const Tilemap&,bool&,const Personaje&);
    void Mover(const Tilemap&);
    void Dibujar() const override;
};
/// @brief Constructor con coordenadas iniciales
/// @param xInicial Posicion x al spawnear
/// @param yInicial Posicion 'y' al spawnear
Enemigo::Enemigo(const int& xInicial,const int& yInicial) : Personaje(xInicial,yInicial){}

/// @brief Encargada de la logica de movimiento
/// @param tilemap El tilemap principal
void Enemigo::Mover(const Tilemap& tilemap)
{
    static unsigned short framesCounter = 0;
    framesCounter++;
    if (framesCounter >= FRAMES_PARA_UPDATE)
    {
        // Movimiento aleatorio
        int cambioX = GetRandomValue(-1, 1);
        int cambioY = GetRandomValue(-1, 1);
        
        do {// Si comentas este do-while, sigue funcionando el movimiento, pero el enemigo tiende a ir mas hacia los bordes
            cambioX = GetRandomValue(-1, 1);
            cambioY = GetRandomValue(-1, 1);
        } while (Tilemap::ObtenerTile(posicion.x + cambioX, posicion.y + cambioY, tilemap, false) == Tilemap::TILE_BORDE);
        
        
		if (Tilemap::ObtenerTile(posicion.x + cambioX, posicion.y + cambioY, tilemap, false) != Tilemap::TILE_BORDE) {
			posicion.x += static_cast<float>(cambioX);
			posicion.y += static_cast<float>(cambioY);
		}
        framesCounter = 0;
    }
}
/// @brief Actualizacion cada frame del Enemigo
/// @param tilemap El tilemap principal
/// @param gameOver Variable de flujo del juego GameOver
/// @param pers El jugador
void Enemigo::Update(const Tilemap &tilemap,bool& gameOver,const Personaje& pers)
{
    // En caso de que ya hayamos llenado los tiles o los enemigos nos mataron, no actualizamos
    if (gameOver)
        return;

    // Movimiento
    Mover(tilemap);

    // Si el enemigo esta en un tile de relleno o borde, lo teleportamos a una posicion aleatoria.
    // Esto para que si el jugador lo encierra, el enemigo siga participando en el juego
    while (Tilemap::ObtenerTile(static_cast<int>(posicion.x), static_cast<int>(posicion.y), tilemap, false) == Tilemap::TILE_RELLENO || Tilemap::ObtenerTile(static_cast<int>(posicion.x), static_cast<int>(posicion.y), tilemap, false) == Tilemap::TILE_BORDE)
    {
        posicion.x = GetRandomValue(1, Tilemap::ANCHO - 1);
        posicion.y = GetRandomValue(1, Tilemap::ALTO_MAPA_JUGABLE - 1);
    }

    // Si el enemigo esta en la misma posicion del jugador, es game over
    if (this->posicion == pers.GetPosition())
        gameOver = true;
    
    // Si el enemigo esta tocando un borde no-endurecido, es game over
    if (Tilemap::ObtenerTile(posicion.x, posicion.y, tilemap, false) == Tilemap::TILE_LINEA_INCOMPLETA)
        gameOver = true;
}

void Enemigo::Dibujar()const{
	Rectangle rectSpritePers = { 14.0f * TAMANIO_TILE, 20.0f * TAMANIO_TILE, TAMANIO_TILE, TAMANIO_TILE };
	DrawTextureRec(tilesheet, rectSpritePers, { posicion.x * TAMANIO_TILE, posicion.y * TAMANIO_TILE }, WHITE);
}


//-------------------------------------------------------------------------------------------------

int main(void)
{
    // Initialization
	//--------------------------------------------------------------------------------------
	const int screenWidth = 800;
	const int screenHeight = 480;

    
    bool gameOver = false;
    bool victoria = false;
	Personaje pers;
	Tilemap tmap;
    Enemigo enemigo(3,5);

	InitWindow(screenWidth, screenHeight, "QIX Sebastian L.");

	SetTargetFPS(60); // Set our game to run at 60 frames-per-second
	//--------------------------------------------------------------------------------------
	// Main game loop

    // Cargamos los assets necesarios
	pers.CargaTextura("Assets/Tilesheet/colored-transparent_packed.png");
	tmap.CargaTextura("Assets/Tilesheet/colored-transparent_packed.png");
    enemigo.CargaTextura("Assets/Tilesheet/colored-transparent_packed.png");
    

    while (!WindowShouldClose()) // Detect window close button or ESC key
	{

		// Update
		//----------------------------------------------------------------------------------
		// TODO: Update your variables here
		//----------------------------------------------------------------------------------

        pers.Update(tmap,gameOver);
        enemigo.Update(tmap,gameOver,pers);
        
        // Revisamos el porcentaje de mapa dibujado para el fin del juego
        tmap.RevisaMapa(gameOver,victoria);

        // Draw
		//----------------------------------------------------------------------------------
		BeginDrawing();

		ClearBackground(BLACK);
        
		tmap.Dibujar();
		pers.Dibujar();
        enemigo.Dibujar();

        
        // Pantalla de victoria/derrota
        if (victoria && gameOver)
            DrawText("Felicidades. GANASTE", 20, 400, 50, GOLD);
        else if (!victoria && gameOver)
            DrawText("Perdiste. Intentalo de nuevo :(", 20, 400, 50, GOLD);


        EndDrawing();
		//----------------------------------------------------------------------------------
	}

	// De-Initialization
	//--------------------------------------------------------------------------------------

	CloseWindow(); // Close window and OpenGL context
	//--------------------------------------------------------------------------------------



	return 0;
}

# Cómo Hostear el Emulador en GitHub Pages

## Paso 1: Crear un Repositorio en GitHub

1. Ve a [github.com](https://github.com) y haz login
2. Haz click en el botón **"New"** (verde) para crear un nuevo repositorio
3. Nombre sugerido: `mellotron-web-emulator`
4. Marca como **Public** (necesario para GitHub Pages gratis)
5. **NO** inicialices con README (ya tienes archivos)
6. Click en **"Create repository"**

## Paso 2: Subir tus Archivos

Abre una terminal en la carpeta del proyecto y ejecuta:

```bash
# Inicializar Git (si no lo has hecho ya)
git init

# Añadir todos los archivos necesarios
git add web_emulator.html
git add samples/
git add README.md

# Hacer el primer commit
git commit -m "Initial commit: Mellotron Web Emulator"

# Conectar con tu repositorio de GitHub (reemplaza TU_USUARIO)
git remote add origin https://github.com/TU_USUARIO/mellotron-web-emulator.git

# Subir a GitHub
git branch -M main
git push -u origin main
```

## Paso 3: Activar GitHub Pages

1. Ve a tu repositorio en GitHub
2. Click en **Settings** (arriba a la derecha)
3. En el menú lateral, click en **Pages**
4. En "Source", selecciona **"Deploy from a branch"**
5. En "Branch", selecciona **main** y carpeta **/ (root)**
6. Click en **Save**

## Paso 4: Esperar y Acceder

- GitHub tardará 1-2 minutos en construir tu sitio
- Tu emulador estará disponible en:
  ```
  https://TU_USUARIO.github.io/mellotron-web-emulator/web_emulator.html
  ```

## Notas Importantes

### ⚠️ Tamaño de los Samples
GitHub tiene un límite de **100MB por archivo** y **1GB por repositorio**.

Si tus samples pesan mucho:
- **Opción A**: Comprime los WAV a menor bitrate (16-bit 44.1kHz mono)
- **Opción B**: Usa un CDN externo (como Cloudflare R2 o AWS S3) para los samples
- **Opción C**: Incluye solo 2-3 instrumentos en el repo público

### 🔧 Actualizar el Código

Cada vez que hagas cambios:

```bash
git add .
git commit -m "Descripción de los cambios"
git push
```

GitHub Pages se actualizará automáticamente en ~1 minuto.

## Alternativa: Netlify (Más Potente)

Si GitHub Pages te da problemas con el tamaño:

1. Ve a [netlify.com](https://netlify.com) y haz login con GitHub
2. Click en **"Add new site" > "Import an existing project"**
3. Selecciona tu repositorio
4. Deploy settings:
   - Build command: (dejar vacío)
   - Publish directory: `/`
5. Click en **Deploy**

Netlify te dará una URL tipo `https://random-name-123.netlify.app`

## Dominio Personalizado (Opcional)

Si tienes un dominio propio:
- En GitHub Pages: Settings > Pages > Custom domain
- En Netlify: Site settings > Domain management

---

¿Necesitas ayuda con algún paso específico?

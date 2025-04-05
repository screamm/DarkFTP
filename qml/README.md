# DarkFTP - Glassmorfism UI

Detta är en modern, glassmorfism-inspirerad användargränssnitt för DarkFTP-applikationen.

## Komponenter

### Grundkomponenter

- **GlassmorphismStyle.qml** - Baskomponent som skapar glastransparens-effekten
- **main.qml** - Huvudlayout för applikationen
- **FileListView.qml** - Listvisning för filer
- **NavigationBar.qml** - Toppnavigation
- **TransferPanel.qml** - Panel för filöverföringar
- **ConnectionDialog.qml** - Dialog för serveranslutning

## Glassmorfism-effekten

Glassmorfism är en modernt UI-trend som efterliknar frostat glas med följande egenskaper:
- Genomskinlighet/transparens
- Oskärpa (blur)
- Subtil gräns
- Djupeffekt med skuggor

Implementationen använder QML och QtGraphicalEffects för att skapa dessa visuella effekter.

## Integration med C++

För att integrera detta UI med backend-koden behöver du:

1. Exponera C++-klasser till QML via registerType
2. Skapa modeller för listvy-komponenterna
3. Koppla signaler mellan C++-koden och QML-gränssnittet

## Färganpassning

Färgtemat kan enkelt anpassas genom att ändra följande värden:
- Bakgrundsfärg: `#121212` i main.qml
- Panelfärg: tintColor-egenskapen i GlassmorphismStyle-instanser (ex. `"#202530"`)
- Knappgradienter: `"#4070c0"` till `"#2050a0"` för blå knappar

## Bakgrundsbild

För att glassmorfism-effekten ska fungera optimalt behöver du lägga en bakgrundsbild med namnet `blur_bg.jpg` i "images"-mappen. Se README.txt i images-mappen för detaljer. 
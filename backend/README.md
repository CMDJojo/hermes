# Installera
Vi använder C++23. Bygg gärna via Cmake. För att installera dependencies på windows använder 
vi [vcpkg](https://vcpkg.io/en/getting-started.html). Följ instruktionerna på deras hemsida för att ladda ner,
kör sedan
```
vcpkg install boost
```
Notera att vcpkg inte ligger på path som standard så man får gå in i katalogen där 
binärfilen ligger. För att Cmake ska hitta filerna behöver även följande flagga läggas till:

```
-DCMAKE_TOOLCHAIN_FILE=[path to vcpkg]/scripts/buildsystems/vcpkg.cmake 
```
Om du använder CLion, öppna Settings > Build, Execution, Deployment > Cmake.

På macOS kan `boost` installeras via Homebrew: `brew install boost`.

# En bättre livsstilskarta
Vid användning:
* Välj en plats (avgränsning: en hållplats) från karta
* Alla personer som bor inom en radie från denna summeras till ett genomsnitt som beskriver "livskvalitet", där det ingår:
  * Genomsnittlig pendlingstid till jobb
  * Genomsnittlig försämring i pendlingstid i jämförelse med bil

# Att implementera
* Från hållplats A till alla andra punkter/hållplatser
  - Lägg till stöd för gång/cykel/bil etc. för jämförelser
  - Kan lägga till tester till en interaktiv karta redan nu?
* Ta en person, och skapa relevant levnadsförhållandedata (index?)
  - Kommer behöva anropa funktioner från dijkstra-programmet
* Hitta vilka personer som bor nära en "punkt".
  - En grej att börja testa med om man vill är att visualisera alla personer på en karta för att börja se med.


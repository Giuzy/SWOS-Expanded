# SWOS Expanded 0.10.38 — Importazione iniziale

Questo archivio va **estratto direttamente nella cartella del repository GitHub già clonato**, non in una sottocartella. Mantieni i file `.github/` e `README.md` già presenti nel repository.

Include sorgenti, Makefile, risorse e dati originali della 0.10.38, oltre ai BAT di avvio e SDL2.dll. I testi storici della root sono raccolti in `PROJECT_HISTORY.md`; le vecchie anteprime sono in `source_images/legacy_previews/`. Log, file temporanei ed eseguibili compilati sono esclusi.

Non è una nuova build, non è stato verificato un avvio su Windows e non è stato modificato il codice di gioco. Il workflow GitHub Actions presente nel repository deve ancora essere testato.

Dopo l'estrazione, da Git Bash nella cartella del repository:

```bash
git status --short
git add .
git commit -m "Import SWOS Expanded 0.10.38 sources and assets"
git push origin main
```

Se `git push` segnala un problema, copia il messaggio di errore senza cancellare nulla.

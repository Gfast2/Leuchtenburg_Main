# Leuchtenburg_Main
Project Waagschalspiel. Main Programm on main controller.

Log & Bug fix:
2016-3-30:
/ Nach der Benutzung, wenn Waage inactive für mehr als 30 Sekunde nicht weiter gespielt, würde es automatisch wieder start. Vavriable ist "Waagezeit_Konstante".
/ Bei ”Zuschwor„ Situation würde es ab sofort noch mal starten (Calibriren). Es ist c.a. 6 Schaufe Sand.
/ Motoren von 2 bis 4 auf Motor Leistung 60% hoch eingestellt. Motor 1 auf 65% eingestellt.
/ Sound Track Volume (1-18, Tone der Herr Böttinger) der vorne Waage auf -9 eingestellt, hintere auf -13 bleibt.

2016-3-2
/ soundAmb(int) method (Tab „SoundRoutine”) wird auskommentiert, weil the wave trigger board immer überlastet. 
/ const int Schwellwert von 200 bis zu 250 (Tab: „Main”) eingestellt, um die vierte Schale früher zu stoppen.
/ G_Max_Calib von 50 auf 100. Das bedeutet die Waage schafft jetzt bevor überlastung noch mehr.




TODO:
/ Die vierte Schale sollte beim Erfolg stelle immer automatisch zu der richtige position gehen.
/ die soundAmb(int amb) sollte andres programmieren, dadurch wir später die Ambient Geräusche haben dürfen.

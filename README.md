# Patch USB Ghost-Control Manba V2 pour Ghost control / StonedModder



La base reste celle de Ghost-Control de StonedModder : lire une manette USB supportee, traduire
ses rapports d'input, creer une manette virtuelle PS5, puis injecter des
`ScePadData`.

Le travail ajoute ici concerne surtout :

- le support Manba V2 en USB ;
- le mode Manba PC/XInput ;
- le mode Manba Switch USB/dongle ;
- la correction de l'axe Y en mode Switch Manba ;
- le comportement propre entre la Manba et la manette officielle quand elles
  essayent de prendre le meme user ;
- Un peu de notes de recherche Bluetooth separees, car le Bluetooth n'est pas resolu
  dans cette ELF, encore en recherche...

## Credits

- Projet/tool original : StonedModder,
  `Ghostcontrol-PS5-USB-Controller-Patcher`
  - https://github.com/StonedModder/Ghostcontrol-PS5-USB-Controller-Patcher
- Tests Manba V2 NBJr USB, validation PS5, verification des axes, tests
  changement user et tests Manba/officielle .
- PS5 SDK John tromblom 

## Contenu Du Dossier

```text
ELF/
  GhostControl-Cleanup.elf
  GhostControl-ManbaV2-NBJr-USB-Patch.elf

SOURCE MODIFIEE PAYLOAD/
  Sources modifiees utilisees pour compiler la payload finale testee OK

Launch-GhostControl-ManbaV2-NBJr.bat
Launch-GhostControl-ManbaV2-NBJr.ps1

```

## Ce Qui Fonctionne En USB

- Manba V2 en mode PC/XInput USB.
- Manba V2 en mode Switch USB/dongle.
- Correction de l'axe Y en mode Switch Manba dans `payload/gc_main.c`.
- Quand la Manba devient active, la manette officielle se deconnectee proprement.
- Si la manette officielle est rallumee sur un autre user, les deux manettes
  peuvent rester actives pour jouer a deux.
- Si la manette officielle est rallumee sur le meme user que la Manba, la VDA
  Manba est liberee et l'officielle reprend la main.

## Recherche Bluetooth

Pendant des tests, la PS5 voyait bien la partie Bluetooth de la Manba, mais elle
se comportait comme un accessoire, pas comme une vraie manette `scePad`
utilisable. La popup profil/user pouvait parfois s'ouvrir, et la console pouvait
afficher deux manettes/accessoires, mais le chemin Bluetooth Manba ne donnait
pas un flux d'input stable

Points vus pendant les tests :

- Le transport Bluetooth etait visible comme device MediaTek :

```text
/dev/ugen0.2
VID:PID 0x0e8d:0x3603
manufacturer="MediaTek Inc."
product="Wireless_Device"
class=0xe0 sub=0x01 proto=0x01
```

- Ce device MediaTek indique le transport/adaptateur Bluetooth, pas les boutons
  de la manette.
- Le mode receiver/update de la Manba a aussi ete vu en `1a34:f517`.
- D'autres devices comme Realtek `0x0bda:0x9210` sont du bruit USB/adaptateur et
  ne doivent pas etre pris pour une manette.
- En USB, la payload a un vrai device `/dev/ugen*` et de vrais reports input sur
  l'endpoint `0x81`.
- En Bluetooth, on n'a pas obtenu le meme chemin de reports input lisibles.
- La manette officielle devient une vraie manette avec handles `scePad` et user.
- La Manba Bluetooth est restee sur un chemin accessoire, pas un pad `scePad`
  fiable.

Endroits recherches  :

- scan `/dev/ugen*` ;
- descriptors USB ;
- endpoints USB ;
- lignes klog `Open Pad` ;
- lignes klog `DEVICE_ADDED` ;
- ids MBus physiques finissant par `0x0300` ;
- ids virtuels crees par la payload ;
- `scePadInit` ;
- `scePadGetHandle` ;
- `scePadVirtualDeviceAddDevice` ;
- `scePadVirtualDeviceInsertData` ;
- `scePadVirtualDeviceDeleteDevice` ;
- `scePadSetProcessPrivilege` ;
- `SceShellUI` ;
- `SceShellCore` ;
- `libScePad` ;
- `libSceMbus` ;
- `sceMbusDisconnectDevice` ;
- `sceMbusBindDeviceWithUserId`.

Ce qui a servi dans le patch USB final :

- deconnexion/liberation de la manette officielle physique ;
- detection quand l'officielle reprend le meme user ;
- liberation de la VDA Manba quand l'officielle reprend ce user.

Ce qui n'est pas resolu :

- binder la Manba Bluetooth comme vraie manette ;
- lire de vrais reports input Bluetooth Manba ;
- assigner la Manba Bluetooth a un user comme une manette `scePad` normale.

 La Manba BT est vue par la PS5, mais elle reste bloquee avant le
chemin input jeu.

Points trouves :

- Adresse BT Manba confirmee :
  - normale : `98:b6:ea:bd:cd:58`
  - reverse en memoire : `58 cd bd ea b6 98`
- Dans `SceSysCore` / `SceMbusKmodEventPolling`, un event Manba a ete retrouve :
  - hit Manba reverse autour de `+0x1ff30`
  - debut event estime autour de `+0x1ff10`
  - signature event : `0x08 / 0x04 / 0x03`
  - `manba_hits_total=1`
  - `manba_hits_heap=0`
- Dans `SceMbusHeap`, la DualSense officielle apparait avec son BT et son
  VID/PID, mais la Manba n'a pas d'entree pad equivalente.
- La comparaison event officielle vs event Manba montre que l'event officiel
  porte BT + VID/PID ensemble, alors que l'event Manba porte la MAC mais pas de
  VID/PID Manba proche. Cela pointe vers un probleme de classification/promotion
  accessoire vers pad, pas vers un simple patch d'adresse.
- Les essais , et les tests
  `KMOD_TO_HEAP … ont confirme que ces pistes ne suffisent pas :
  - patch VID/PID ShellUI seul ;
  - patch SIG8 ShellUI ;
  - bind direct `0x190300` ;
  - bind direct `0x30300` ;
  - force `0x2030e` comme pad ;
  - fallback `0x2030e -> 0x190300` ;
  - activation table ShellUI `active20/user24/type28` avec ou sans user ;
  - scans simples `/dev/hid` et `/dev/bluetooth_hid`.
- Les tests  jeu montrent que `eboot.bin` a sa propre table `libScePad` :
  l'officielle y est active, mais la Manba BT reste cote Shell/Cdlg/accessoire.
- Le jeu appelle `scePadRead`.
  ...

Conclusion actuelle : le Bluetooth demande une recherche separee autour de la
pile Bluetooth PS5, HCI/L2CAP/HIDP, la logique accessoire vers pad, ou les
modules/PRX utilises par la DualSense officielle.


## Lancer La Payload Finale

1. Demarrer l'ecoute payload sur la PS5, port `9021`.
2. Lancer :

```powershell
.\Launch-GhostControl-ManbaV2-NBJr.ps1
```

ou double-cliquer :

```text
Launch-GhostControl-ManbaV2-NBJr.bat
```

3. Entrer l'IP de la PS5.
4. Le launcher envoie d'abord `GhostControl-Cleanup.elf`, attend 2 secondes,
   puis envoie `GhostControl-ManbaV2-NBJr-USB-Patch.elf`.


## Ou Regarder Pour Les Patchs

Le guide detaille est ici :

```text
PATCH_MANBA_FINAL_DETAIL.txt
```

Les zones importantes :

- `payload/controller_mamba.h`
  - VID/PID Manba.
  - mode PC/XInput.
  - mode Switch USB.
- `payload/controller_mamba.c`
  - mapping Manba PC/XInput.
  - axes XInput.
  - boutons XInput.
- `payload/gc_main.c`
  - detection Manba.
  - routage USB.
  - correction axe Y Switch Manba apres `nintendo_handle_packet()`.
  - logique de liberation/recreation VDA.
  - detection manette officielle qui reprend le meme user.
- `payload/shellui_pad.c`
  - fonctions ShellUI/MBus pour couper une manette physique.
  - verification handle/user.
- `payload/shellui_pad.h`
  - declarations des nouvelles fonctions.
- `payload/Makefile`
  - ajout de `controller_mamba.o`.

## Correction Switch Importante

La correction Switch n'est pas dans `controller_nintendo.c`.

Elle est dans :

```text
payload/gc_main.c
```

dans `usb_hid_thread`, apres :

```c
injected = nintendo_handle_packet(...);
```

avec :

```c
if (injected > 0 && is_mamba_switch)
    pad.leftStick.y = (uint8_t)(255u - pad.leftStick.y);
```

Pourquoi ici ?

Parce que si on inverse directement dans `controller_nintendo.c`, on risque de
casser les vraies manettes Switch Pro ou les autres pads compatibles Nintendo ( je n'ai pas de manettes pour test )
La correction doit rester limitee a la Manba reconnue comme `057e:2009`.

## Build

La compilation utilise l'environnement PS5 payload SDK…

Script utilise pendant les tests :

```text
payload/build_correction.sh
```

ELF finale testee OK :

```text
ELF/GhostControl-ManbaV2-NBJr-USB-Patch.elf
```

## Avertissement

Recherche uniquement a titre educatif . A utiliser a vos risques et perils . Tester uniquement avec mon environnement PS5 6.02 et Manba.
Je ne dispose pas d'outils , script ou payload magique universel malheuresement pour analyse plus approfondi pour support d autre manettes ou adapation BT,  je procede par etapes par etapes, cela demande du temps ...


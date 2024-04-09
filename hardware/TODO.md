# Changes to make for Rev C board

- [ ] Fix grounding issue on 10A adc channel
- [ ] Add silkscreen reference designators for all parts
- [ ] Figure out why peak detectors/current sense amps won't quite hit 3V
  - [ ] Might require redesign of peak detectors as the Schottky diodes have a
        max 4V reverse so we probably can't run it all off 5V (or can we?) Maybe
        there is a different Schottky with higher max reverse bias voltage?

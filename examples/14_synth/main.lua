local synth = sn.Synth.new()
synth:setBpm(130)
synth:setStepBeats(0.25) -- 16th note steps (1.0 = quarter note)
synth:setADSR(0.002, 0.06, 0.3, 0.12)
synth:setMasterGain(0.9)

-- Pattern syntax (minimal):
-- - Notes: c4, d#3, a5, or MIDI numbers like 60
-- - Rests: ~ or .
-- - Subdivision: [c4 e4 g4] fits into one step
-- - Modifiers: :durBeats @vel ^wave #cutoffHz !pan
--   ex) c4^saw@0.8:0.5#1200!-0.2
synth:setPattern(
"[c4 e4 g4 c5] ~ [c3^saw@0.6#900!-0.3 ~ g2^saw@0.6#900!0.3 ~] [c4^square@0.4 e4^square@0.4 g4^square@0.4 c5^square@0.4]")
synth:play()

function update()
    if sn.Keyboard.isPressed(sn.Keyboard.ESCAPE) then
        synth:stop()
        sn.Script.load("main", ".")
    end
    if sn.Keyboard.isPressed(sn.Keyboard.SPACE) then
        synth:play()
    end
end

function draw()
    sn.Graphics.drawText("Synth (miniaudio) - press SPACE to restart, ESC to go back", FONT, sn.Vec2.new(0),
        sn.Color.new(1), 16)
    sn.Graphics.drawText("Pattern uses [], ~, and modifiers (^wave @vel :dur #cutoff !pan)", FONT, sn.Vec2.new(0, -20),
        sn.Color.new(1), 14)
end

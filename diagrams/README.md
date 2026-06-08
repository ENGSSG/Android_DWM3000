# Diagrams

## Entry / Exit Protocol (per tracked person)

State machine that governs how a UWB-tagged person is added to and removed from the face-blur
tracking system. Implemented in the `DWM3000_Android_tracker` submodule,
`FacePredictionTracker.kt` (`OwnerLockState`). See [../DATA_FLOW.md](../DATA_FLOW.md) for the full
narrative and requirements mapping.

![Entry/Exit protocol state machine](entry_exit_protocol.svg)

Fill colour = blur state:
- **gray** — no blur (`IDLE`, `PENDING`, `CONFIRMING`, `EXITED`)
- **red** — blur ON, confirmed/coasting (`CONFIRMED`, `COASTING`)
- **amber** — blur ON, visual recovery (`VISUAL_LOST`, `REACQUIRING`)

### Files
| File | What |
|------|------|
| `entry_exit_protocol.dot` | Graphviz source (authoritative for the rendered image). |
| `entry_exit_protocol.svg` / `.png` | Rendered diagram (PNG is 140 dpi). |
| `entry_exit_protocol.mmd` | Mermaid source (renders inline on GitHub; mirrors `DATA_FLOW.md`). |

### Regenerate

```bash
cd diagrams
dot -Tsvg entry_exit_protocol.dot -o entry_exit_protocol.svg
dot -Tpng -Gdpi=140 entry_exit_protocol.dot -o entry_exit_protocol.png
```

(Graphviz `dot` only — no extra tooling needed.)

### Mermaid version (GitHub-rendered)

```mermaid
stateDiagram-v2
    [*] --> IDLE

    IDLE --> PENDING: UWB live + faces present
    PENDING --> CONFIRMING: best match passes cost / margin / >=MIN_ASSOCIATION_PAIRS
    CONFIRMING --> PENDING: candidate destabilizes
    CONFIRMING --> CONFIRMED: stable x ENTRY_CONFIRM_FRAMES (3) -> blur ON

    CONFIRMED --> VISUAL_LOST: bound face occluded (UWB live)
    VISUAL_LOST --> REACQUIRING: candidate stable
    REACQUIRING --> CONFIRMED: x REACQUIRE_CONFIRM_FRAMES (3)
    REACQUIRING --> VISUAL_LOST: candidate lost
    VISUAL_LOST --> CONFIRMED: same track reappears

    CONFIRMED --> COASTING: UWB dropout (peer leaves live set)
    VISUAL_LOST --> COASTING: UWB dropout
    REACQUIRING --> COASTING: UWB dropout
    COASTING --> CONFIRMED: UWB resumes (face present)
    COASTING --> VISUAL_LOST: UWB resumes (face gone)
    COASTING --> EXITED: coast age > UWB_LOSS_GRACE_NS (2 s)

    PENDING --> EXITED: UWB dropout (never confirmed)
    CONFIRMING --> EXITED: UWB dropout (never confirmed)
    EXITED --> PENDING: re-entry < REID_MEMORY_NS (5 s), REID_CONFIRM_FRAMES (1)
    EXITED --> [*]
```

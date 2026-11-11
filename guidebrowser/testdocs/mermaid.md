# Mermaid test page

Three real diagrams of how the studio works, drawn natively. The text before and after each one is ordinary page text.

## How a page arrives

```mermaid
graph TD
    A[Link clicked] --> B{File exists?}
    B -->|yes| C[Read the file]
    B -->|no| D([Show: No such file])
    C --> E{Markdown?}
    E -->|yes| F[parseMarkdown]
    E -->|no| G[parseCode]
    F --> H((Layout))
    G --> H
    H --> I[Draw and upload]
```

That was a top-down flowchart with a diamond, a stadium and a circle.

## The pipeline, left to right

```mermaid
flowchart LR
    src[Mermaid text] --> parse[parser]
    parse --> graph[Graph]
    parse --> seq[SequenceDiagram]
    graph --> layout[layout and route]
    seq --> lay2[layoutSequence]
    layout --> px[Pixels]
    lay2 --> px
    px --> page([The page])
```

## A click on a link

```mermaid
sequenceDiagram
    participant V as Visitor
    participant P as DocPanel
    participant F as Files
    participant R as Renderer
    V->>P: click on a link
    Note over V,P: the ray meets the page
    P->>P: resolve the path
    P->>F: read the file
    F-->>P: text
    P->>R: draw the page
    Note right of R: one draw, then upload
    R-->>V: new page on the wall
```

And a short one after it:

```mermaid
sequenceDiagram
    Jon->>Studio: open the page
    Studio-->>Jon: the diagram
```

The end of the page.

#include "sky/skyPrivate.hpp"
#include "sky/skyRendererUtils.hpp"
#include "sky/skyVertex.hpp"

void RenderList::Clear() {
  m_queuedCount = 0;
}

void RenderList::AddRender(
  VertexRender *vertexRender
) {
  u32 idx = 0;

  if (!vertexRender)
    return;

  if (!m_capacity)
    goto Error;

  while (m_vertexRender[idx]) {
    idx++;
    if (idx == m_capacity) {
Error:
      AssertMsg(false, "RenderList \"%s\" exceeded maximum capacity %d", m_name, m_capacity);
      return;
    }
  }

  m_vertexRender[idx] = vertexRender;
}

void RenderList::RemoveRender(
  VertexRender *vertexRender
) {
  for (u32 i = 0; i < m_capacity; ++i) {
    if (m_vertexRender[i] == vertexRender)
      m_vertexRender[i] = nullptr;
  }

  VertexRender **begin = m_queuedRender
    , **end = begin + m_queuedCount;

  VertexRender **it = begin;
  while (it != end && *it != vertexRender) {
    it++;
  }

  if (it != end) {
    VertexRender **write = it;
    VertexRender **read  = it + 1;
    while (read != end) {
      if (*read != vertexRender) {
        *write = *read;
        write++;
      }
      read++;
    }
    end = write;
  }

  m_queuedCount = static_cast<u32>(end - begin);
}

void RenderList::EnqueueRender(
  VertexRender *vertexRender
) {
  Assert(vertexRender->IsInitialized());

  m_queuedCount++;
  m_queuedRender[m_queuedCount] = vertexRender;
}

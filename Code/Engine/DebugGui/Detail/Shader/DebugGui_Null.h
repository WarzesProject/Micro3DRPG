#ifdef RENDERER_NULL
	if (renderer.getNameId() == Renderer::NameId::NULL_DUMMY)
	{
		vertexShaderSourceCode = fragmentShaderSourceCode = "42";
	}
	else
#endif
{
	// Error! (unsupported renderer)
	RENDERER_LOG(mRendererRuntime.getContext(), CRITICAL, "The renderer backend \"%s\" isn't supported by the renderer runtime debug GUI", renderer.getName())
}

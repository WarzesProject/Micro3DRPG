#ifdef RENDERER_NULL
	if (renderer.getNameId() == Renderer::NameId::NULL_DUMMY)
	{
		vertexShaderSourceCode = fragmentShaderSourceCode = "42";
	}
	else
#endif
{
	// Error! (unsupported renderer)
	RENDERER_LOG(rendererRuntime.getContext(), CRITICAL, "The renderer backend \"%s\" isn't supported by the renderer runtime VR hidden area mesh compositor pass", renderer.getName())
}

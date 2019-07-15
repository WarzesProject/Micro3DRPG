#ifdef RENDERER_NULL
	if (renderer->getNameId() == Renderer::NameId::NULL_DUMMY)
	{
		vertexShaderSourceCode = fragmentShaderSourceCode = "42";
	}
	else
#endif
{
	// Error! (unsupported renderer)
	RENDERER_LOG(renderer->getContext(), CRITICAL, "Error: Unsupported renderer")
}

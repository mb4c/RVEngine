class RenderStats
{
public:
	static RenderStats &GetInstance()
	{
		static RenderStats instance;
		return instance;
	}
	int DrawCalls = 0;

private:
	RenderStats() = default;

	~RenderStats() = default;

	RenderStats(const RenderStats &) = delete;

	RenderStats &operator=(const RenderStats &) = delete;
};

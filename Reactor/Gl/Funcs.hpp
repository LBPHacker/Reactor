#pragma once
#include "Defs.hpp"

#define REACTOR_GLFUNC_LIST(X) \
	X(void, glAttachShader, GLuint program, GLuint shader) \
	X(void, glBindBufferBase, GLenum target, GLuint index, GLuint buffer) \
	X(void, glBindBuffer, GLenum target, GLuint buffer) \
	X(void, glBindVertexArray, GLuint array) \
	X(void, glClearColor, GLfloat red, GLfloat green, GLfloat blue, GLfloat alpha) \
	X(void, glClear, GLbitfield mask) \
	X(void, glCompileShader, GLuint shader) \
	X(void, glCreateBuffers, GLsizei n, GLuint *buffers) \
	X(GLuint, glCreateProgram, void) \
	X(GLuint, glCreateShader, GLenum shaderType) \
	X(void, glCreateVertexArrays, GLsizei n, GLuint *arrays) \
	X(void, glDebugMessageCallback, glDebugMessageCallbackFunc callback, const void *userParam) \
	X(void, glDebugMessageControl, GLenum source, GLenum type, GLenum severity, GLsizei count, const GLuint *ids, GLboolean enabled) \
	X(void, glDeleteBuffers, GLsizei n, const GLuint *buffers) \
	X(void, glDeleteProgram, GLuint program) \
	X(void, glDeleteShader, GLuint shader) \
	X(void, glDeleteVertexArrays, GLsizei n, const GLuint *arrays) \
	X(void, glDispatchCompute, GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z) \
	X(void, glDrawArrays, GLenum mode, GLint first, GLsizei count) \
	X(void, glEnable, GLenum cap) \
	X(void, glEnableVertexArrayAttrib, GLuint vaobj, GLuint index) \
	X(void, glFinish, void) \
	X(void, glGenBuffers, GLsizei n, GLuint *buffers) \
	X(GLint, glGetAttribLocation, GLuint program, const GLchar *name) \
	X(void, glGetIntegerv, GLenum pname, GLint *data) \
	X(void, glGetProgramInfoLog, GLuint program, GLsizei maxLength, GLsizei *length, GLchar *infoLog) \
	X(void, glGetProgramiv, GLuint program, GLenum pname, GLint *params) \
	X(GLuint, glGetProgramResourceIndex, GLuint program, GLenum programInterface, const char *name) \
	X(void, glGetProgramResourceiv, GLuint program, GLenum programInterface, GLuint index, GLsizei propCount, const GLenum *props, GLsizei bufSize, GLsizei *length, GLint *params) \
	X(void, glGetShaderInfoLog, GLuint shader, GLsizei maxLength, GLsizei *length, GLchar *infoLog) \
	X(void, glGetShaderiv, GLuint shader, GLenum pname, GLint *params) \
	X(GLuint, glGetUniformBlockIndex, GLuint program, const GLchar *uniformBlockName) \
	X(GLint, glGetUniformLocation, GLuint program, const GLchar *name) \
	X(void, glLinkProgram, GLuint program) \
	X(void, glMemoryBarrier, GLbitfield barriers) \
	X(void, glNamedBufferData, GLuint buffer, GLsizeiptr size, const void *data, GLenum usage) \
	X(void, glNamedBufferSubData, GLuint buffer, GLintptr offset, GLsizeiptr size, const void *data) \
	X(void, glProgramUniform1f, GLuint program, GLint location, GLfloat v0) \
	X(void, glProgramUniform1i, GLuint program, GLint location, GLint v0) \
	X(void, glProgramUniform1ui, GLuint program, GLint location, GLuint v0) \
	X(void, glProgramUniform3i, GLuint program, GLint location, GLint v0, GLint v1, GLint v2) \
	X(void, glProgramUniformMatrix3fv, GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value) \
	X(void, glShaderSource, GLuint shader, GLsizei count, const GLchar **string, const GLint *length) \
	X(void, glShaderStorageBlockBinding, GLuint program, GLuint storageBlockIndex, GLuint storageBlockBinding) \
	X(void, glUniformBlockBinding, GLuint program, GLuint uniformBlockIndex, GLuint uniformBlockBinding) \
	X(void, glUseProgram, GLuint program) \
	X(void, glVertexArrayAttribFormat, GLuint vaobj, GLuint attribindex, GLint size, GLenum type, GLboolean normalized, GLuint relativeoffset) \
	X(void, glVertexArrayVertexBuffer, GLuint vaobj, GLuint bindingindex, GLuint buffer, GLintptr offset, GLsizei stride) \
	X(void, glViewport, GLint x, GLint y, GLsizei width, GLsizei height) \
	// last line of the macro, don't remove

namespace Reactor::Gl
{
	using glDebugMessageCallbackFunc = void (*)(
		GLenum source,
		GLenum type,
		GLuint id,
		GLenum severity,
		GLsizei length,
		const GLchar *message,
		const void *userParam
	);

#define REACTOR_GLFUNC_DECLARE(ret, name, ...) extern ret (*name)(__VA_ARGS__);
	REACTOR_GLFUNC_LIST(REACTOR_GLFUNC_DECLARE)
#undef REACTOR_GLFUNC_DECLARE

	void LoadGlFuncs();
	void UnloadGlFuncs();
}

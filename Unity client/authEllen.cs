using System.Collections;
using System.Collections.Generic;
using UnityEngine;

public class authEllen : MonoBehaviour {

    protected CharacterController controller;
    protected Animator animator;
    readonly int m_HashGrounded = Animator.StringToHash("Grounded");
    private void Awake()
    {
        controller = GetComponent<CharacterController>();
        animator = GetComponent<Animator>();
    }

	// Update is called once per frame
	void Update () {
        
        Vector3 movement = new Vector3(0, -1f, 0);
        controller.Move(movement);
        animator.SetBool(m_HashGrounded, true);
    }
}
